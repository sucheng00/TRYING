#include "imu.h"
#include "BMI088driver.h"
#include "FreeRTOS.h"
#include "cmsis_os.h"
#include "easyflash.h"
#include "imu_heater.h"
#include "logging.h"
#include "madgwick.h"
#include "microros.h"
#include "timers.h"
#include <math.h>
#include <rmw_microros/time_sync.h>

#define CALIBRATION_EF_KEY "imu_calibration"
#define IMU_TARGET_TEMPERATURE 40.0
#define IMU_CALIBRATION_TEMPERATURE_TOLERANCE 1.0
#define IMU_CALIBRATION_SAMPLES 300
#define IMU_UPDATE_HZ 200
#define MADGWICK_BETA 0.1

#include "usart.h"
#include "stdio.h"
#ifdef __GNUC__
#define PUTCHAR_PROTOTYPE int __io_putchar(int ch)
#else
#define PUTCHAR_PROTOTYPE int fputc(int ch, FILE *f)
#endif

PUTCHAR_PROTOTYPE
{
    HAL_UART_Transmit(&huart1 , (uint8_t *)&ch, 1, 0xFFFF);
    return ch;
}
int _write(int file, char *ptr, int len)

{
      int DataIdx;
      for (DataIdx = 0; DataIdx < len;DataIdx++)
     {
           __io_putchar(*ptr++);
     }
      return len;
}


static float imu_rotation_matrix[3][3] = {
    {0.0f, 1.0f, 0.0f}, {-1.0f, 0.0f, 0.0f}, {0.0f, 0.0f, 1.0f}};

static pid_controller_param_t heater_pid_param = {
    .mode = PID_CONTROLLER_POSITION,
    .k_p = 800,
    .k_i = 10,
    .k_d = 0,
    .integral_limit = 8000,
    .out_limit = 20000,
};

static imu_calibration_t calibration;
static pid_controller_t heater_pid;

static float gyro_raw[3];
static float accel_raw[3];

static imu_data_t data;

// 读取 BMI088 加速度、角速度、温度，并记录时间戳
static void read_bmi088_data(void) {
	data.epoch_ns = rmw_uros_epoch_nanos();
	BMI088_Read(gyro_raw, accel_raw, &data.temperature);
}

// 执行一次温度控制，请按 IMU_UPDATE_HZ 频率调用
static void run_temperature_control(void) {
	pid_controller_update(&heater_pid, data.temperature,
	                      IMU_TARGET_TEMPERATURE);
	uint16_t pwm;
	if (heater_pid.out < 0) {
		pwm = 0;
	} else {
		pwm = heater_pid.out;
	}
	imu_heater_set_pwm(pwm);
}

bool imu_read_stored_calibration(imu_calibration_t *result) {
	size_t read_len;
	ef_get_env_blob(CALIBRATION_EF_KEY, result, sizeof(imu_calibration_t),
	                &read_len);
	if (read_len != sizeof(imu_calibration_t)) {
		return false;
	} else {
		return true;
	}
}

void imu_write_stored_calibration(const imu_calibration_t *result) {
	if (ef_set_env_blob(CALIBRATION_EF_KEY, result,
	                    sizeof(imu_calibration_t)) != EF_NO_ERR) {
		LOG_ERROR("Failed to save IMU calibration result");
		for (;;)
			;
	}
}

static void imu_do_calibration(void) {
	// 等待 IMU 到达设定温度
	for (;;) {
		read_bmi088_data();
		run_temperature_control();

		if (fabsf(data.temperature - IMU_TARGET_TEMPERATURE) <
		    IMU_CALIBRATION_TEMPERATURE_TOLERANCE)
			break;

		HAL_Delay(1000 / IMU_UPDATE_HZ);
	}

	LOG_INFO("IMU temperature reached %f", data.temperature);

	// 校准陀螺仪
	calibration.gx = 0;
	calibration.gy = 0;
	calibration.gz = 0;

	for (int i = 0; i < IMU_CALIBRATION_SAMPLES; i++) {
		read_bmi088_data();
		run_temperature_control();

		calibration.gx += gyro_raw[0];
		calibration.gy += gyro_raw[1];
		calibration.gz += gyro_raw[2];

		HAL_Delay(1000 / IMU_UPDATE_HZ);
	}

	calibration.gx /= IMU_CALIBRATION_SAMPLES;
	calibration.gy /= IMU_CALIBRATION_SAMPLES;
	calibration.gz /= IMU_CALIBRATION_SAMPLES;

	LOG_INFO("IMU calibrated");

	// 存储校准数据
	imu_write_stored_calibration(&calibration);
}

// 解算加速度、角速度（作用旋转矩阵并校准）
static void solve_gyro_accel(void) {
	data.gx = gyro_raw[0] * imu_rotation_matrix[0][0] +
	          gyro_raw[1] * imu_rotation_matrix[0][1] +
	          gyro_raw[2] * imu_rotation_matrix[0][2] - calibration.gx;
	data.gy = gyro_raw[0] * imu_rotation_matrix[1][0] +
	          gyro_raw[1] * imu_rotation_matrix[1][1] +
	          gyro_raw[2] * imu_rotation_matrix[1][2] - calibration.gy;
	data.gz = gyro_raw[0] * imu_rotation_matrix[2][0] +
	          gyro_raw[1] * imu_rotation_matrix[2][1] +
	          gyro_raw[2] * imu_rotation_matrix[2][2] - calibration.gz;
	data.ax = accel_raw[0] * imu_rotation_matrix[0][0] +
	          accel_raw[1] * imu_rotation_matrix[0][1] +
	          accel_raw[2] * imu_rotation_matrix[0][2];
	data.ay = accel_raw[0] * imu_rotation_matrix[1][0] +
	          accel_raw[1] * imu_rotation_matrix[1][1] +
	          accel_raw[2] * imu_rotation_matrix[1][2];
	data.az = accel_raw[0] * imu_rotation_matrix[2][0] +
	          accel_raw[1] * imu_rotation_matrix[2][1] +
	          accel_raw[2] * imu_rotation_matrix[2][2];
}

void imu_task(const void *arg) {
	// 初始化 BMI088
	if (BMI088_Init() != BMI088_NO_Error) {
		LOG_ERROR("BMI088 initialization failed");
		for (;;)
			;
	}

	// 初始化加热
	imu_heater_init();
	pid_controller_init(&heater_pid, &heater_pid_param);

	// 读取校准数据
	if (imu_read_stored_calibration(&calibration)) {
		LOG_INFO("Read IMU calibration result from flash");
	} else {
		// 未校准过，进行校准
		LOG_INFO("Start IMU calibration");
		imu_do_calibration();
	}

	// 初始位姿
	data.orientation.w = 1.0;
	data.orientation.x = 0.0;
	data.orientation.y = 0.0;
	data.orientation.z = 0.0;

	TickType_t xLastWakeTime;
	xLastWakeTime = xTaskGetTickCount();

	for (;;) {
		read_bmi088_data();
		run_temperature_control();

		solve_gyro_accel();
		madgwick_update_6d(&data.orientation, MADGWICK_BETA,
		                   1.0 / IMU_UPDATE_HZ, data.gx, data.gy, data.gz,
		                   data.ax, data.ay, data.az);

		microros_publish_imu_data(&data);

		float yaw, pitch, roll;
		quaternion_to_rpy(&data.orientation, &yaw, &pitch, &roll);
		// LOG_INFO("yaw: %f, pitch: %f, roll: %f", yaw * 180.0 / M_PI,
		//          pitch * 180.0 / M_PI, roll * 180.0 / M_PI);
		// printf("yaw: %f, pitch: %f, roll: %f\n", yaw * 180.0 / M_PI,
		//          pitch * 180.0 / M_PI, roll * 180.0 / M_PI);
		
		vTaskDelayUntil(&xLastWakeTime, pdMS_TO_TICKS(1000 / IMU_UPDATE_HZ));
	}
}
