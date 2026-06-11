/**
 * @file    tim.c
 * @brief   定时器驱动模块 - 实现
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 使用 TIM2 通用定时器实现 10ms 周期性中断:
 * - 时钟源: APB1 定时器时钟 = 72MHz (APB1=36MHz, 但 APB1 分频器≠1 时定时器时钟×2)
 * - 预分频器: 7200-1 → 定时器计数时钟 = 10kHz
 * - 自动重装载: 100-1 → 中断周期 = 100/10kHz = 10ms
 * - 中断服务: 更新 g_ms_counter, 调用 Key_Scan() 执行按键消抖
 *
 * 知识点覆盖: 通用定时器配置、定时器中断、NVIC 优先级管理
 */

/* ============================ 头文件包含 ============================ */
#include "tim.h"
#include "key.h"

/* ============================ 全局变量 ============================ */
volatile uint32_t g_ms_counter = 0;  /* 全局毫秒计数器 (10ms 分辨率) */

/* ============================ 函数实现 ============================ */

/**
 * @brief  初始化 TIM2 定时器 (10ms 周期)
 * @note   配置详解:
 *         - 定时器时钟 = 72MHz (APB1 定时器时钟)
 *         - 预分频器 = 7200 → 计数器时钟 = 10kHz
 *         - 周期 = 100 → 中断频率 = 100Hz = 10ms
 *         - NVIC 优先级 = 2 (低于 USART1 的 1, 高于 I2C 的 3)
 */
void TIM2_Init(void)
{
    TIM_ClockConfigTypeDef   sClockSourceConfig = {0};
    TIM_MasterConfigTypeDef  sMasterConfig = {0};

    /* ---- TIM2 基本配置 ---- */
    htim2.Instance               = TIM2;
    htim2.Init.Prescaler         = TIM2_PRESCALER;    /* 7199: 分频 7200 */
    htim2.Init.CounterMode       = TIM_COUNTERMODE_UP;
    htim2.Init.Period            = TIM2_PERIOD;       /* 99: 周期 100 */
    htim2.Init.ClockDivision     = TIM_CLOCKDIVISION_DIV1;
    htim2.Init.AutoReloadPreload = TIM_AUTORELOAD_PRELOAD_ENABLE;

    if (HAL_TIM_Base_Init(&htim2) != HAL_OK)
    {
        System_Error_Handler();
    }

    /* ---- 时钟源: 内部时钟 (72MHz APB1 Timer Clock) ---- */
    sClockSourceConfig.ClockSource = TIM_CLOCKSOURCE_INTERNAL;
    if (HAL_TIM_ConfigClockSource(&htim2, &sClockSourceConfig) != HAL_OK)
    {
        System_Error_Handler();
    }

    /* ---- 主从模式: 禁用 ---- */
    sMasterConfig.MasterOutputTrigger = TIM_TRGO_RESET;
    sMasterConfig.MasterSlaveMode     = TIM_MASTERSLAVEMODE_DISABLE;
    if (HAL_TIMEx_MasterConfigSynchronization(&htim2, &sMasterConfig) != HAL_OK)
    {
        System_Error_Handler();
    }

    /* ---- 启动定时器, 使能中断 ---- */
    HAL_TIM_Base_Start_IT(&htim2);
}

/**
 * @brief  TIM2 周期中断回调
 * @param  htim 定时器句柄指针
 * @note   每 10ms 由硬件中断触发:
 *         1. g_ms_counter += 10 (累加 10ms)
 *         2. 调用 Key_Scan() 执行按键扫描消抖
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        g_ms_counter += 10;       /* 累加器 (每 10ms) */
        Key_Scan();               /* 按键扫描与消抖   */
    }
}

/**
 * @brief  获取系统上电以来的毫秒计数
 * @return uint32_t 毫秒计数值 (分辨率 10ms)
 */
uint32_t TIM_Get_Millis(void)
{
    uint32_t ms;

    /* 原子读取 (32位对齐访问在 Cortex-M3 上是原子的) */
    ms = g_ms_counter;

    return ms;
}

/**
 * @brief  基于定时器的毫秒级阻塞延时
 * @param  ms 延时毫秒数
 * @note   不依赖 SysTick, 作为备用延时方案
 *         精度 ±10ms, 最小延时 10ms
 */
void TIM_Delay_Ms(uint32_t ms)
{
    uint32_t start = TIM_Get_Millis();

    while ((TIM_Get_Millis() - start) < ms)
    {
        /* 轮询等待 */
        __NOP();
    }
}
