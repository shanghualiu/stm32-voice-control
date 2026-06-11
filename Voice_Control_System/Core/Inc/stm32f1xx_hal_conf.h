/**
 * @file    stm32f1xx_hal_conf.h
 * @brief   STM32F1xx HAL 库配置文件
 * @author  STMicroelectronics (adapted for Voice Control System)
 * @date    2026-06-11
 *
 * 本文件用于配置 HAL 库各模块的使能/禁用, 以及相关参数设置。
 * 根据项目需求选择性使能外设模块, 减小编译后的固件体积。
 */

#ifndef __STM32F1XX_HAL_CONF_H
#define __STM32F1XX_HAL_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ 模块使能开关 ============================ */
#define HAL_MODULE_ENABLED          /* HAL 核心模块            */
#define HAL_GPIO_MODULE_ENABLED     /* GPIO 模块               */
#define HAL_DMA_MODULE_ENABLED      /* DMA 模块                */
#define HAL_RCC_MODULE_ENABLED      /* RCC 时钟模块            */
#define HAL_FLASH_MODULE_ENABLED    /* Flash 模块              */
#define HAL_PWR_MODULE_ENABLED      /* 电源管理模块            */
#define HAL_CORTEX_MODULE_ENABLED   /* Cortex 系统模块         */

#define HAL_I2C_MODULE_ENABLED      /* I2C 模块 (OLED 通信)    */
#define HAL_TIM_MODULE_ENABLED      /* 定时器模块 (按键消抖)   */
#define HAL_UART_MODULE_ENABLED     /* USART 模块 (LD3320)     */

/* 本设计未使用的模块 (禁用以减小体积) */
/* #define HAL_ADC_MODULE_ENABLED   */
/* #define HAL_CAN_MODULE_ENABLED   */
/* #define HAL_SPI_MODULE_ENABLED   */
/* #define HAL_RTC_MODULE_ENABLED   */
/* #define HAL_WWDG_MODULE_ENABLED  */
/* #define HAL_IWDG_MODULE_ENABLED  */

/* ============================ 振荡器配置 ============================ */
#if !defined(HSE_VALUE)
#define HSE_VALUE              8000000U   /* 外部高速晶振 8MHz        */
#endif

#if !defined(HSI_VALUE)
#define HSI_VALUE              8000000U   /* 内部高速振荡器 8MHz      */
#endif

#if !defined(HSE_STARTUP_TIMEOUT)
#define HSE_STARTUP_TIMEOUT    100U       /* HSE 启动超时 (ms)        */
#endif

#if !defined(LSI_VALUE)
#define LSI_VALUE              40000U     /* 内部低速振荡器 40kHz     */
#endif

#if !defined(LSE_VALUE)
#define LSE_VALUE              32768U     /* 外部低速晶振 32.768kHz   */
#endif

#if !defined(LSE_STARTUP_TIMEOUT)
#define LSE_STARTUP_TIMEOUT    5000U      /* LSE 启动超时 (ms)        */
#endif

/* ============================ 系统配置 ============================ */
#define VDD_VALUE              3300U      /* 供电电压 3.3V            */
#define TICK_INT_PRIORITY      0U         /* SysTick 中断优先级       */
#define USE_RTOS               0U         /* 不使用 RTOS              */
#define PREFETCH_ENABLE        1U         /* 使能 Flash 预取          */

/* ============================ 断言配置 ============================ */
/* 调试时建议开启, 发布时关闭以减小体积 */
/* #define USE_FULL_ASSERT       1U      */

#ifdef USE_FULL_ASSERT
#define assert_param(expr)      ((expr) ? (void)0U : assert_failed((uint8_t *)__FILE__, __LINE__))
void assert_failed(uint8_t* file, uint32_t line);
#else
#define assert_param(expr)      ((void)0U)
#endif

/* ============================ 头文件包含 ============================ */
/* HAL 核心头文件 */
#include "stm32f1xx.h"

/* 根据使能的模块包含对应头文件 */
#ifdef HAL_RCC_MODULE_ENABLED
#include "stm32f1xx_hal_rcc.h"
#endif

#ifdef HAL_GPIO_MODULE_ENABLED
#include "stm32f1xx_hal_gpio.h"
#endif

#ifdef HAL_DMA_MODULE_ENABLED
#include "stm32f1xx_hal_dma.h"
#endif

#ifdef HAL_CORTEX_MODULE_ENABLED
#include "stm32f1xx_hal_cortex.h"
#endif

#ifdef HAL_FLASH_MODULE_ENABLED
#include "stm32f1xx_hal_flash.h"
#endif

#ifdef HAL_PWR_MODULE_ENABLED
#include "stm32f1xx_hal_pwr.h"
#endif

#ifdef HAL_I2C_MODULE_ENABLED
#include "stm32f1xx_hal_i2c.h"
#endif

#ifdef HAL_TIM_MODULE_ENABLED
#include "stm32f1xx_hal_tim.h"
#endif

#ifdef HAL_UART_MODULE_ENABLED
#include "stm32f1xx_hal_uart.h"
#endif

#ifdef __cplusplus
}
#endif

#endif /* __STM32F1XX_HAL_CONF_H */
