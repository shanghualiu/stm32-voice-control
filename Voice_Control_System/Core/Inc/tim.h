/**
 * @file    tim.h
 * @brief   定时器驱动模块 - 头文件
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 使用 TIM2 通用定时器实现周期性中断:
 * - 定时周期: 10ms (用于按键扫描消抖)
 * - 定时器时钟: 72MHz (APB1 定时器时钟)
 * - 预分频器: 7200-1 → 时钟变为 10kHz
 * - 自动重装载: 100-1 → 中断周期 = 100/10kHz = 10ms
 *
 * 知识点覆盖: 定时器中断、NVIC 优先级配置
 */

#ifndef __TIM_H
#define __TIM_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ 头文件包含 ============================ */
#include "main.h"

/* ============================ 定时器配置宏 ============================ */
#define TIM2_PRESCALER    7199           /* 预分频: 72MHz/(7199+1)=10kHz */
#define TIM2_PERIOD       99             /* 自动重装载: 10kHz/(99+1)=100Hz=10ms */
#define TIM2_IRQ_PRIORITY 2              /* 中断优先级 (0最高)         */
#define TIM2_IRQ_SUB_PRI  0              /* 中断子优先级               */

/* ============================ 系统滴答外部变量 ============================ */
extern volatile uint32_t g_ms_counter;   /* 全局毫秒计数器             */

/* ============================ 函数原型 ============================ */

/**
 * @brief  初始化 TIM2 定时器
 * @note   配置为 10ms 周期中断, 用于按键扫描与系统计时
 *         使能 TIM2 中断并配置 NVIC 优先级
 */
void TIM2_Init(void);

/**
 * @brief  TIM2 周期中断回调函数
 * @param  htim 定时器句柄指针
 * @note   每 10ms 调用一次:
 *         1. 更新全局毫秒计数器 (g_ms_counter += 10)
 *         2. 调用 Key_Scan() 执行按键扫描与消抖
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim);

/**
 * @brief  获取系统运行毫秒数
 * @return uint32_t 系统上电以来的毫秒计数
 * @note   基于 g_ms_counter, 分辨率 10ms
 */
uint32_t TIM_Get_Millis(void);

/**
 * @brief  毫秒级阻塞延时 (不依赖 SysTick)
 * @param  ms 延时毫秒数
 * @note   基于 g_ms_counter 轮询实现, 精度 ±10ms
 *         仅在 SysTick 不可用时使用
 */
void TIM_Delay_Ms(uint32_t ms);

#ifdef __cplusplus
}
#endif

#endif /* __TIM_H */
