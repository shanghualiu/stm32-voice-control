###############################################################################
# 基于离线语音识别的家电控制系统 - Makefile (ARM GCC)
#
# 工具链: arm-none-eabi-gcc (GNU Arm Embedded Toolchain)
# 目标芯片: STM32F103C8T6 (Blue Pill)
# 内核: Cortex-M3 (ARMv7-M)
#
# === 三种使用方式 ===
#
# 方式1: GitHub Actions (推荐, 零安装)
#   git push → GitHub Actions 自动编译 → 下载 hex 产物
#
# 方式2: 本地编译 (需安装 ARM GCC)
#   bash fetch_hal_deps.sh    # 下载 HAL/CMSIS 依赖
#   make clean && make        # 编译
#
# 方式3: Wokwi 在线编译
#   将所有文件打包上传到 Wokwi, 使用其内置编译器
###############################################################################

# ---- 项目名称 ----
TARGET = Voice_Control_System

# ---- 工具链 ----
CROSS    ?= arm-none-eabi-
CC        = $(CROSS)gcc
AS        = $(CROSS)gcc
LD        = $(CROSS)gcc
OBJCOPY   = $(CROSS)objcopy
SIZE      = $(CROSS)size

# ---- 目录结构 (适配自动下载的扁平结构) ----
BUILD_DIR   = build
CORE_DIR    = Core
DRIVERS_DIR = Drivers

# CMSIS 和 HAL 文件都放在 Drivers/ 根层 (GitHub Actions 下载后的结构)
# 本地安装 STM32CubeF1 的用户可自行调整路径

# ---- 芯片定义 ----
CPU       = -mcpu=cortex-m3
MCU       = $(CPU) -mthumb -msoft-float

# ---- 链接脚本 ----
LDSCRIPT  = STM32F103C8Tx_FLASH.ld

# ---- 自动扫描源文件 (适配扁平结构) ----
# Core 应用层源文件
C_SOURCES  = $(wildcard $(CORE_DIR)/Src/*.c)

# HAL 库源文件 (Drivers/STM32F1xx_HAL_Driver/*.c)
C_SOURCES += $(wildcard $(DRIVERS_DIR)/STM32F1xx_HAL_Driver/*.c)

# CMSIS 系统文件 (Drivers/CMSIS/*.c)
C_SOURCES += $(wildcard $(DRIVERS_DIR)/CMSIS/*.c)

# 启动文件 (Drivers/CMSIS/*.s)
ASM_SOURCES = $(wildcard $(DRIVERS_DIR)/CMSIS/*.s)

# 如果启动文件未找到, 尝试标准 STM32Cube 路径
ifeq ($(strip $(ASM_SOURCES)),)
    ASM_SOURCES = $(wildcard $(DRIVERS_DIR)/CMSIS/Device/ST/STM32F1xx/Source/Templates/gcc/*.s)
endif
ifeq ($(strip $(C_SOURCES)),)
    # 备用: 标准 STM32CubeF1 路径
    C_SOURCES += $(wildcard $(DRIVERS_DIR)/STM32F1xx_HAL_Driver/Src/*.c)
    C_SOURCES += $(wildcard $(DRIVERS_DIR)/CMSIS/Device/ST/STM32F1xx/Source/Templates/*.c)
endif

# ---- 头文件路径 ----
INCLUDES  = -I$(CORE_DIR)/Inc
INCLUDES += -I$(DRIVERS_DIR)/STM32F1xx_HAL_Driver
INCLUDES += -I$(DRIVERS_DIR)/CMSIS

# 备用标准路径
INCLUDES += -I$(DRIVERS_DIR)/STM32F1xx_HAL_Driver/Inc
INCLUDES += -I$(DRIVERS_DIR)/CMSIS/Include
INCLUDES += -I$(DRIVERS_DIR)/CMSIS/Device/ST/STM32F1xx/Include

# ---- 编译选项 ----
DEFINES  = -DSTM32F103xB
DEFINES += -DUSE_HAL_DRIVER

CFLAGS   = $(MCU)
CFLAGS  += $(DEFINES)
CFLAGS  += $(INCLUDES)
CFLAGS  += -Wall -Wextra -Wno-unused-parameter -Wno-missing-field-initializers
CFLAGS  += -Os -g3
CFLAGS  += -ffunction-sections -fdata-sections
CFLAGS  += -fno-common
CFLAGS  += -std=gnu11

ASFLAGS  = $(MCU) $(INCLUDES) $(DEFINES) -x assembler-with-cpp

LDFLAGS  = $(MCU)
LDFLAGS += -specs=nano.specs -specs=nosys.specs
LDFLAGS += -T$(LDSCRIPT)
LDFLAGS += -Wl,--gc-sections
LDFLAGS += -Wl,-Map=$(BUILD_DIR)/$(TARGET).map,--cref
LDFLAGS += -Wl,--print-memory-usage
LDFLAGS += -lm -lc -lnosys

# ---- 目标文件 ----
OBJECTS  = $(addprefix $(BUILD_DIR)/,$(notdir $(C_SOURCES:.c=.o)))
OBJECTS += $(addprefix $(BUILD_DIR)/,$(notdir $(ASM_SOURCES:.s=.o)))

vpath %.c $(sort $(dir $(C_SOURCES)))
vpath %.s $(sort $(dir $(ASM_SOURCES)))

# ---- 默认目标 ----
.PHONY: all clean flash setup help

all: check-deps $(BUILD_DIR)/$(TARGET).hex $(BUILD_DIR)/$(TARGET).bin
	@echo ""
	@echo "=========================================="
	@echo "  Build SUCCESS!"
	@echo "  HEX: $(BUILD_DIR)/$(TARGET).hex"
	@echo "  BIN: $(BUILD_DIR)/$(TARGET).bin"
	@echo "=========================================="

# ---- 检查依赖是否存在 ----
check-deps:
	@if [ ! -d "$(DRIVERS_DIR)/STM32F1xx_HAL_Driver" ] || \
	    [ -z "$$(ls $(DRIVERS_DIR)/STM32F1xx_HAL_Driver/*.c 2>/dev/null)" ]; then \
		echo ""; \
		echo "⚠  HAL 库文件未找到!"; \
		echo "  请先运行: bash fetch_hal_deps.sh"; \
		echo "  或执行:   make setup"; \
		echo ""; \
		exit 1; \
	fi

# ---- 下载 HAL/CMSIS 依赖 ----
setup:
	@echo "正在下载 STM32F1 HAL 库和 CMSIS 依赖..."
	bash fetch_hal_deps.sh

# ---- 编译规则 ----
$(BUILD_DIR)/%.o: %.c | $(BUILD_DIR)
	@echo "CC  $(notdir $<)"
	@$(CC) -c $(CFLAGS) $< -o $@

$(BUILD_DIR)/%.o: %.s | $(BUILD_DIR)
	@echo "AS  $(notdir $<)"
	@$(AS) -c $(ASFLAGS) $< -o $@

$(BUILD_DIR)/$(TARGET).elf: $(OBJECTS)
	@echo "LD  $(notdir $@)"
	@$(LD) $(LDFLAGS) $(OBJECTS) -o $@
	@echo ""
	@$(SIZE) $@

$(BUILD_DIR)/%.hex: $(BUILD_DIR)/%.elf
	@echo "HEX $(notdir $@)"
	@$(OBJCOPY) -O ihex $< $@

$(BUILD_DIR)/%.bin: $(BUILD_DIR)/%.elf
	@echo "BIN $(notdir $@)"
	@$(OBJCOPY) -O binary $< $@

$(BUILD_DIR):
	@mkdir -p $(BUILD_DIR)

clean:
	@rm -rf $(BUILD_DIR)
	@echo "Clean done."

flash: $(BUILD_DIR)/$(TARGET).hex
	@echo "Flash not available in Wokwi - upload hex file manually."
	@echo "Hex file location: $(BUILD_DIR)/$(TARGET).hex"

help:
	@echo ""
	@echo "===== 离线语音识别家电控制系统 - 编译指南 ====="
	@echo ""
	@echo "  方式1 (GitHub Actions, 推荐):"
	@echo "    1. git push 到 GitHub"
	@echo "    2. Actions → Build → 下载 hex 产物"
	@echo ""
	@echo "  方式2 (本地编译):"
	@echo "    1. make setup       (下载依赖, 仅需执行一次)"
	@echo "    2. make clean && make"
	@echo ""
	@echo "  方式3 (已安装 STM32CubeF1):"
	@echo "    1. 修改 Makefile 中的路径指向你的 CubeF1 目录"
	@echo "    2. make clean && make"
	@echo ""
	@echo "  输出文件:"
	@echo "    build/Voice_Control_System.hex  (Wokwi 用)"
	@echo "    build/Voice_Control_System.bin  (Flash 用)"
	@echo "    build/Voice_Control_System.elf  (调试用)"
	@echo ""
