.PHONY: all clean download build_microros

######################################
# target
######################################
TARGET = chassis


######################################
# building variables
######################################
# debug build?
DEBUG = 0
# optimization
ifeq ($(DEBUG), 1)
	OPT = -Og -ggdb
	BUILD_TYPE = Debug
else
	OPT = -O2 -ggdb
	BUILD_TYPE = RelWithDebInfo
endif


#######################################
# paths
#######################################
# Build path
BUILD_DIR = build

# micro-ros
UROS_DEP = micro-ros/src/install/.dirstamp
UROS_LIB = $(BUILD_DIR)/libmicroros.a

######################################
# source
######################################
# C sources
C_SOURCE_DIRS = application Core Drivers Middlewares USB_DEVICE bmi088 easyflash bsp
C_SOURCES = $(foreach dir,$(C_SOURCE_DIRS),$(shell find $(dir) -type f -name "*.c"))

# ASM sources
ASM_SOURCES =  \
startup_stm32f407xx.s


#######################################
# binaries
#######################################
PREFIX = arm-none-eabi-
# The gcc compiler bin path can be either defined in make command via GCC_PATH variable (> make GCC_PATH=xxx)
# either it can be added to the PATH environment variable.
ifdef GCC_PATH
CC = $(GCC_PATH)/$(PREFIX)gcc
AS = $(GCC_PATH)/$(PREFIX)gcc -x assembler-with-cpp
CP = $(GCC_PATH)/$(PREFIX)objcopy
SZ = $(GCC_PATH)/$(PREFIX)size
AR = $(GCC_PATH)/$(PREFIX)ar
TOOLCHAIN_PREFIX = $(GCC_PATH)/$(PREFIX)
else
CC = $(PREFIX)gcc
AS = $(PREFIX)gcc -x assembler-with-cpp
CP = $(PREFIX)objcopy
SZ = $(PREFIX)size
AR = $(PREFIX)ar
TOOLCHAIN_PREFIX = $(PREFIX)
endif
HEX = $(CP) -O ihex
BIN = $(CP) -O binary -S

#######################################
# CFLAGS
#######################################
# cpu
CPU = -mcpu=cortex-m4

# fpu
FPU = -mfpu=fpv4-sp-d16

# float-abi
FLOAT-ABI = -mfloat-abi=hard

# mcu
MCU = $(CPU) -mthumb $(FPU) $(FLOAT-ABI)

# macros for gcc
# AS defines
AS_DEFS =

# C defines
C_DEFS =  \
-DUSE_HAL_DRIVER \
-DSTM32F407xx \
-DPLATFORM_NAME_FREERTOS \
-DRCUTILS_LOG_MIN_SEVERITY=RCUTILS_LOG_MIN_SEVERITY_INFO \
-DRCL_COMMAND_LINE_ENABLED


# AS includes
AS_INCLUDES =  \
-ICore/Inc

# C includes
C_INCLUDE_DIRS =  \
application/include \
bsp/boards/Inc \
Core/Inc \
USB_DEVICE/App \
USB_DEVICE/Target \
Drivers/STM32F4xx_HAL_Driver/Inc \
Drivers/STM32F4xx_HAL_Driver/Inc/Legacy \
Middlewares/Third_Party/FreeRTOS/Source/include \
Middlewares/Third_Party/FreeRTOS/Source/CMSIS_RTOS \
Middlewares/Third_Party/FreeRTOS/Source/portable/GCC/ARM_CM4F \
Middlewares/ST/STM32_USB_Device_Library/Core/Inc \
Middlewares/ST/STM32_USB_Device_Library/Class/CDC/Inc \
Drivers/CMSIS/Device/ST/STM32F4xx/Include \
Drivers/CMSIS/Include \
bmi088/include \
easyflash/inc 

C_INCLUDES = $(foreach dir,$(C_INCLUDE_DIRS),-I$(dir))


# compile gcc flags
ASFLAGS = $(MCU) $(AS_DEFS) $(AS_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

CFLAGS = $(MCU) $(C_DEFS) $(C_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

ifeq ($(DEBUG), 1)
CFLAGS += -g -gdwarf-2
endif

# Generate dependency information
CFLAGS += -MMD -MP -MF"$(@:%.o=%.d)"


#######################################
# LDFLAGS
#######################################
# link script
LDSCRIPT = STM32F407IGHx_FLASH.ld

# libraries
LIBS = -lc -lm -lnosys
LIBDIR =
LDFLAGS = $(MCU) -specs=nano.specs -u _printf_float -T$(LDSCRIPT) $(LIBDIR) $(LIBS) -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref -Wl,--gc-sections -fwhole-program -u _printf_float

# default action: build all
all: $(BUILD_DIR)/$(TARGET).elf $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin


#######################################
# build the application
#######################################
# list of objects
OBJECTS = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
vpath %.c $(sort $(dir $(C_SOURCES)))
# list of ASM program objects
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

$(BUILD_DIR)/%.o: %.c $(UROS_DEP) | $(BUILD_DIR)
	$(CC) -c $(CFLAGS) -Wa,-a,-ad,-alms=$(BUILD_DIR)/$(notdir $(<:.c=.lst)) $< -o $@

$(BUILD_DIR)/%.o: %.s $(UROS_DEP) | $(BUILD_DIR)
	$(AS) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS) $(UROS_LIB)
	$(CC) -Wl,--start-group $(OBJECTS) $(UROS_LIB) -Wl,--end-group $(LDFLAGS) -o $@
	$(SZ) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(HEX) $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf | $(BUILD_DIR)
	$(BIN) $< $@

$(BUILD_DIR):
	mkdir $@

#######################################
# clean up
#######################################
clean:
	-rm -fR $(BUILD_DIR)
#	-rm -rf micro-ros/dev/cloned micro-ros/src/cloned
#	-rm -rf micro-ros/dev/install micro-ros/dev/build micro-ros/dev/logs
#	-rm -rf micro-ros/src/install micro-ros/src/build micro-ros/src/logs

#######################################
# dependencies
#######################################
-include $(wildcard $(BUILD_DIR)/*.d)

# *** EOF ***

#######################################
# download
#######################################
OPENOCD = openocd
OPENOCD_CFG = openocd.cfg
OPENOCD_FLASH_START = 0x08000000

download:
	openocd -f $(OPENOCD_CFG) \
	-c init \
	-c "reset halt" \
	-c "flash write_image erase $(BUILD_DIR)/$(TARGET).bin $(OPENOCD_FLASH_START)" \
	-c reset \
	-c shutdown

#######################################
# micro-ros
#######################################
UROS_IGNORED_PKGS = rosidl_typesupport_introspection_cpp rcl_logging_spdlog rclc_examples
UROS_ADDITIONAL_INCLUDES = $(foreach dir,$(C_INCLUDE_DIRS),-I$(realpath $(dir)))
UROS_CFLAGS = $(MCU) $(C_DEFS) $(UROS_ADDITIONAL_INCLUDES) $(OPT) -Wall -fdata-sections -ffunction-sections

UROS_INCLUDE_DIRS = $(shell ./micro-ros/print-include-dirs.sh micro-ros/src/install)
C_INCLUDES += $(foreach dir,$(UROS_INCLUDE_DIRS),-I$(dir))

micro-ros/dev/install/.dirstamp: micro-ros/dev/cloned/.dirstamp
	cd micro-ros/dev && \
	unset CC CXX CFLAGS CXXFLAGS CPPFLAGS LDFLAGS && \
	colcon build --cmake-args -DBUILD_TESTING=OFF && \
	touch install/.dirstamp

micro-ros/dev/cloned/.dirstamp micro-ros/src/cloned/.dirstamp: micro-ros/repos.yaml
	cd micro-ros && \
	vcs import < repos.yaml && \
	touch dev/cloned/.dirstamp && \
	touch src/cloned/.dirstamp

$(UROS_DEP) build_microros: micro-ros/dev/install/.dirstamp micro-ros/src/cloned/.dirstamp
	cd micro-ros/src && \
	rm -rf install && \
	unset AMENT_PREFIX_PATH && \
	PATH=$(subst /opt/ros/$(ROS_DISTRO)/bin,,$(PATH)) && \
	. ../dev/install/local_setup.sh && \
	export TOOLCHAIN_PREFIX="$(TOOLCHAIN_PREFIX)" && \
	export RET_CFLAGS="$(UROS_CFLAGS)" && \
	colcon build \
		--packages-ignore $(UROS_IGNORED_PKGS) \
		--packages-ignore-regex ".*_cpp" ".*_py" \
		--metas ../colcon.meta \
		--cmake-args \
		"--no-warn-unused-cli" \
		-DCMAKE_POSITION_INDEPENDENT_CODE=OFF \
		-DTHIRDPARTY=ON \
		-DBUILD_SHARED_LIBS=OFF \
		-DBUILD_TESTING=OFF \
		-DCMAKE_BUILD_TYPE=$(BUILD_TYPE) \
		-DCMAKE_TOOLCHAIN_FILE=$(realpath micro-ros/toolchain.cmake) \
		-DCMAKE_VERBOSE_MAKEFILE=ON \
		-DCMAKE_EXPORT_COMPILE_COMMANDS=ON && \
	touch install/.dirstamp

$(UROS_LIB): $(UROS_DEP) | $(BUILD_DIR)
	rm -f $(UROS_LIB) && \
	$(AR) crsT $(UROS_LIB) $(shell find micro-ros/src/install -mindepth 3 -maxdepth 3 -type f -path '*/lib/*.a' -print0 | tr '\0' ' ')
