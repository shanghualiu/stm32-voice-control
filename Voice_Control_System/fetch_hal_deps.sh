#!/bin/bash
# 下载 STM32F1 HAL 库和 CMSIS 依赖文件
# 本地编译前执行此脚本即可获取所有依赖

set -e

DRIVERS_DIR="Drivers"
mkdir -p "${DRIVERS_DIR}/CMSIS"
mkdir -p "${DRIVERS_DIR}/STM32F1xx_HAL_Driver"

echo "=== 下载 CMSIS Core (ARM Cortex-M3) ==="
CMSIS_CORE="https://raw.githubusercontent.com/STMicroelectronics/cmsis_core/main/Core/Include"
FILES_CORE=("cmsis_compiler.h" "cmsis_gcc.h" "cmsis_version.h" "core_cm3.h" "mpu_armv7.h")

for f in "${FILES_CORE[@]}"; do
    echo "  -> ${f}"
    curl -sL "${CMSIS_CORE}/${f}" -o "${DRIVERS_DIR}/CMSIS/${f}"
done

echo ""
echo "=== 下载 CMSIS Device (STM32F103) ==="
CMSIS_DEV="https://raw.githubusercontent.com/STMicroelectronics/cmsis_device_f1/master"
FILES_DEV_INC=("stm32f103xb.h" "stm32f1xx.h" "system_stm32f1xx.h")
for f in "${FILES_DEV_INC[@]}"; do
    echo "  -> ${f}"
    curl -sL "${CMSIS_DEV}/Include/${f}" -o "${DRIVERS_DIR}/CMSIS/${f}"
done

echo "  -> system_stm32f1xx.c"
curl -sL "${CMSIS_DEV}/Source/Templates/system_stm32f1xx.c" \
    -o "${DRIVERS_DIR}/CMSIS/system_stm32f1xx.c"

echo "  -> startup_stm32f103xb.s"
curl -sL "${CMSIS_DEV}/Source/Templates/gcc/startup_stm32f103xb.s" \
    -o "${DRIVERS_DIR}/CMSIS/startup_stm32f103xb.s"

echo ""
echo "=== 下载 STM32F1xx HAL Driver ==="
HAL_INC="https://raw.githubusercontent.com/STMicroelectronics/stm32f1xx_hal_driver/master/Inc"
HAL_SRC="https://raw.githubusercontent.com/STMicroelectronics/stm32f1xx_hal_driver/master/Src"

HAL_FILES=(
    "stm32f1xx_hal.h"
    "stm32f1xx_hal_def.h"
    "stm32f1xx_hal_cortex.h"
    "stm32f1xx_hal_gpio.h"
    "stm32f1xx_hal_gpio_ex.h"
    "stm32f1xx_hal_rcc.h"
    "stm32f1xx_hal_rcc_ex.h"
    "stm32f1xx_hal_flash.h"
    "stm32f1xx_hal_flash_ex.h"
    "stm32f1xx_hal_pwr.h"
    "stm32f1xx_hal_dma.h"
    "stm32f1xx_hal_i2c.h"
    "stm32f1xx_hal_tim.h"
    "stm32f1xx_hal_tim_ex.h"
    "stm32f1xx_hal_uart.h"
)

echo "  [头文件]"
for f in "${HAL_FILES[@]}"; do
    echo "  -> ${f}"
    curl -sL "${HAL_INC}/${f}" -o "${DRIVERS_DIR}/STM32F1xx_HAL_Driver/${f}"
done

echo ""
echo "  [源文件]"
HAL_SRC_FILES=(
    "stm32f1xx_hal.c"
    "stm32f1xx_hal_cortex.c"
    "stm32f1xx_hal_gpio.c"
    "stm32f1xx_hal_rcc.c"
    "stm32f1xx_hal_flash.c"
    "stm32f1xx_hal_pwr.c"
    "stm32f1xx_hal_dma.c"
    "stm32f1xx_hal_i2c.c"
    "stm32f1xx_hal_tim.c"
    "stm32f1xx_hal_uart.c"
)

for f in "${HAL_SRC_FILES[@]}"; do
    echo "  -> ${f}"
    curl -sL "${HAL_SRC}/${f}" -o "${DRIVERS_DIR}/STM32F1xx_HAL_Driver/${f}"
done

echo ""
echo "=========================================="
echo "  所有 HAL/CMSIS 依赖下载完成!"
echo "  执行 'make clean && make' 即可编译"
echo "=========================================="
