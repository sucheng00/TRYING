#include "microros.h"
#include "logging.h"
#include "usbd_cdc_if.h"
#include <rcl/rcl.h>
#include <rclc/executor.h>
#include <rclc/rclc.h>
#include <rm_robot_msgs/msg/imu.h>
#include <rmw_microros/time_sync.h>
#include <std_msgs/msg/int32.h>

// 简化 ROSIDL_GET_MSG_TYPE_SUPPORT 为 IDL
#define IDL(PkgName, MsgSubfolder, MsgName)                                    \
	ROSIDL_GET_MSG_TYPE_SUPPORT(PkgName, MsgSubfolder, MsgName)

// 简化判断函数返回值是否为 RCL_RET_OK
#define FAIL(x) ((x) != RCL_RET_OK)

// 表示语句不可能失败（一定返回 RCL_RET_OK），若失败则进入死循环
#define CANNOT_FAIL(x)                                                         \
	{                                                                          \
		if FAIL (x) {                                                          \
			for (;;)                                                           \
				;                                                              \
		}                                                                      \
	}

// micro-ROS 的自定义内存分配器
void *microros_allocate(size_t size, void *state);
void microros_deallocate(void *pointer, void *state);
void *microros_reallocate(void *pointer, size_t size, void *state);
void *microros_zero_allocate(size_t number_of_elements, size_t size_of_element,
                             void *state);

// micro-ROS 命令行参数
static const char *args[] = {"--ros-args", "--disable-stdout-logs",
                             "--disable-external-lib-logs"};

// micro-ROS 相关变量
static rcl_allocator_t allocator;
static rclc_support_t support;
static rcl_node_t node;
static rclc_executor_t executor;

// micro-ROS 是否已经初始化且在运行
static volatile bool microros_running = false;

// 发布者 imu_raw
static rcl_publisher_t imu_pub;

// 发布者 out
static rcl_publisher_t out_pub;

// 订阅者 in
static rcl_subscription_t in_sub;
static std_msgs__msg__Int32 in_msg;
static void on_topic_in(const std_msgs__msg__Int32 *msg) {
	rcl_publish(&out_pub, msg, NULL);
}

/**
 * 初始化并运行 micro-ROS 会话，直到出现错误（如 RCL 错误、Agent 重启等）。
 */
static void run_microros_session(void) {

	// 初始化 rclc support，此过程将与 Agent 握手
	if FAIL (rclc_support_init(&support, sizeof(args) / sizeof(char *), args,
	                           &allocator))
		goto exit;
	check_host_restart(); // 清除 host_restarted 标志

	// 初始化日志系统
	if FAIL (rcl_logging_configure(&support.context.global_arguments,
	                               &allocator))
		goto cleanup_support;

	// NTP 时钟同步
	if FAIL (rmw_uros_sync_session(200))
		goto cleanup_logging;

	// 初始化 rclc executor，此处句柄数为下方所有需要句柄数之和
	if FAIL (rclc_executor_init(&executor, &support.context, 1, &allocator))
		goto cleanup_logging;

	// 初始化 mcu_node 节点
	rcl_node_options_t node_options = rcl_node_get_default_options();
	node_options.rosout_qos = rmw_qos_profile_sensor_data;
	if FAIL (rclc_node_init_with_options(&node, "mcu_node", "", &support,
	                                     &node_options))
		goto cleanup_executor;

	if FAIL (rclc_publisher_init_best_effort(
	             &imu_pub, &node, IDL(rm_robot_msgs, msg, Imu), "imu/raw"))
		goto cleanup_node;

	// 初始化发布者 out
	if FAIL (rclc_publisher_init_best_effort(&out_pub, &node,
	                                         IDL(std_msgs, msg, Int32), "out"))
		goto cleanup_imu_pub;

	// 初始化订阅者 in
	if FAIL (rclc_subscription_init_best_effort(
	             &in_sub, &node, IDL(std_msgs, msg, Int32), "in"))
		goto cleanup_out_pub;

	// 设置 in 的回调函数为 on_topic_in
	// 需要 1 个句柄
	if FAIL (rclc_executor_add_subscription(
	             &executor, &in_sub, &in_msg,
	             (rclc_subscription_callback_t)&on_topic_in, ON_NEW_DATA))
		goto cleanup_in_sub;

	// 运行 micro-ROS 会话
	microros_running = true;
	LOG_INFO("micro-ROS initialized");
	for (;;) {

		// 进行一次 spin（等待数据到达或定时器触发）
		rcl_ret_t ret = rclc_executor_spin_some(&executor, executor.timeout_ns);

		if (ret != RCL_RET_OK && ret != RCL_RET_TIMEOUT) {
			// spin 失败，且不是超时造成
			break; // 会话结束
		}

		if (check_host_restart()) {
			// Agent 发生重启，或 USB 断线重连

			// Agent 重启时需要重新初始化
			// micro-ROS，否则将无法与上位机重新建立会话
			break;
		}
	}
	microros_running = false;

	uxTaskGetStackHighWaterMark(NULL);

	// 资源释放
cleanup_in_sub:
	CANNOT_FAIL(rcl_subscription_fini(&in_sub, &node));

cleanup_out_pub:
	CANNOT_FAIL(rcl_publisher_fini(&out_pub, &node));

cleanup_imu_pub:
	CANNOT_FAIL(rcl_publisher_fini(&imu_pub, &node));

cleanup_node:
	CANNOT_FAIL(rcl_node_fini(&node));

cleanup_executor:
	CANNOT_FAIL(rclc_executor_fini(&executor));

cleanup_logging:
	CANNOT_FAIL(rcl_logging_fini());

cleanup_support:
	CANNOT_FAIL(rclc_support_fini(&support));

exit:
	return;
}

void microros_task(const void *arg) {

	// 初始化 USB
	MX_USB_DEVICE_Init();

	// 初始化 XRCE-DDS 信道
	CANNOT_FAIL(rmw_uros_set_custom_transport(
	    true, NULL, usb_transport_open, usb_transport_close,
	    usb_transport_write, usb_transport_read));

	// 初始化 micro-ROS 内存分配器
	allocator = rcutils_get_zero_initialized_allocator();
	allocator.allocate = &microros_allocate;
	allocator.deallocate = &microros_deallocate;
	allocator.reallocate = &microros_reallocate;
	allocator.zero_allocate = &microros_zero_allocate;
	if (!rcutils_set_default_allocator(&allocator)) {
		// 不可能失败
		for (;;)
			;
	}

	// 运行 micro-ROS 会话，若失败则重试
	for (;;)
		run_microros_session();
}

const char *logging_get_logger_name(void) {
	if (!microros_running)
		return NULL;
	return rcl_node_get_logger_name(&node);
}

static ns_to_time_msg(int64_t ns, builtin_interfaces__msg__Time *msg) {
	msg->sec = (int32_t)(ns / 1000000000LL);
	msg->nanosec = (uint32_t)(ns % 1000000000LL);
}

bool microros_publish_imu_data(const imu_data_t *imu) {
	if (!microros_running)
		return;

	rm_robot_msgs__msg__Imu msg;
	ns_to_time_msg(imu->epoch_ns, &msg.stamp);
	msg.ax = imu->ax;
	msg.ay = imu->ay;
	msg.az = imu->az;
	msg.gx = imu->gx;
	msg.gy = imu->gy;
	msg.gz = imu->gz;
	msg.temperature = imu->temperature;
	msg.orientation.w = imu->orientation.w;
	msg.orientation.x = imu->orientation.x;
	msg.orientation.y = imu->orientation.y;
	msg.orientation.z = imu->orientation.z;
	rcl_publish(&imu_pub, &msg, NULL);
}
