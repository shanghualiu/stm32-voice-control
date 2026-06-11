/**
 * @file    stm32f1xx_it.c
 * @brief   中断服务程序 (ISR) 集合
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 本文件包含所有外设中断的 ISR 函数, 与启动文件 (startup_stm32f103xb.s)
 * 中的中断向量表对应。每个 ISR 调用 HAL 库对应的 IRQHandler 函数。
 *
 * 包含的中断:
 * - SysTick_Handler:     系统滴答定时器 (HAL 延时基准)
 * - TIM2_IRQHandler:     TIM2 定时器中断 (按键消抖)
 * - USART1_IRQHandler:   USART1 串口中断 (LD3320 模拟)
 * - I2C1_EV_IRQHandler:  I2C1 事件中断 (OLED 通信)
 * - I2C1_ER_IRQHandler:  I2C1 错误中断 (OLED 通信)
 * - HardFault_Handler:   硬件错误中断 (调试用)
 *
 * 知识点覆盖: 中断向量表、NVIC、中断优先级管理
 */

/* ============================ 头文件包含 ============================ */
#include "main.h"

/* ============================ 外部变量 ============================ */
extern TIM_HandleTypeDef  htim2;
extern UART_HandleTypeDef huart1;
extern I2C_HandleTypeDef  hi2c1;

/* ============================ Cortex-M3 系统中断 ============================ */

/**
 * @brief  NMI 不可屏蔽中断处理
 */
void NMI_Handler(void)
{
    /* 不可屏蔽中断: 通常用于电源掉电检测等 */
}

/**
 * @brief  硬件错误中断处理
 * @note   发生硬件错误 (如非法内存访问) 时进入
 *         在调试模式下可在此设置断点定位问题
 */
void HardFault_Handler(void)
{
    /* 硬件错误: 进入死循环, 方便调试 */
    while (1)
    {
        /* 可通过调试器查看堆栈和寄存器定位错误源 */
    }
}

/**
 * @brief  内存管理错误中断处理
 */
void MemManage_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  总线错误中断处理
 */
void BusFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  用法错误中断处理
 */
void UsageFault_Handler(void)
{
    while (1)
    {
    }
}

/**
 * @brief  系统服务调用 (SVC) 中断处理
 */
void SVC_Handler(void)
{
}

/**
 * @brief  调试监控中断处理
 */
void DebugMon_Handler(void)
{
}

/**
 * @brief  PendSV 可挂起系统服务中断处理
 */
void PendSV_Handler(void)
{
}

/**
 * @brief  SysTick 系统滴答定时器中断处理
 * @note   HAL 库使用 SysTick 作为 HAL_Delay() 的时基
 *         默认每 1ms 产生一次中断 (72MHz / 72000 = 1kHz)
 */
void SysTick_Handler(void)
{
    HAL_IncTick();
}

/* ============================ 外设中断 ============================ */

/**
 * @brief  TIM2 全局中断处理
 * @note   按键消抖定时器, 每 10ms 触发一次
 *         调用 HAL_TIM_IRQHandler → HAL_TIM_PeriodElapsedCallback
 */
void TIM2_IRQHandler(void)
{
    HAL_TIM_IRQHandler(&htim2);
}

/**
 * @brief  USART1 全局中断处理
 * @note   处理串口发送/接收/错误中断
 *         接收中断触发时调用 HAL_UART_RxCpltCallback
 */
void USART1_IRQHandler(void)
{
    HAL_UART_IRQHandler(&huart1);
}

/**
 * @brief  I2C1 事件中断处理
 * @note   处理 I2C 通信事件 (发送/接收就绪等)
 */
void I2C1_EV_IRQHandler(void)
{
    HAL_I2C_EV_IRQHandler(&hi2c1);
}

/**
 * @brief  I2C1 错误中断处理
 * @note   处理 I2C 通信错误 (仲裁丢失、ACK 失败等)
 */
void I2C1_ER_IRQHandler(void)
{
    HAL_I2C_ER_IRQHandler(&hi2c1);
}
