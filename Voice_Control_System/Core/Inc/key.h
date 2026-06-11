/**
 * @file    key.h
 * @brief   6键独立按键扫描模块 - 头文件
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 基于定时器中断的周期性按键扫描, 实现硬件级消抖。
 * 按键扫描周期: 10ms (由 TIM2 中断驱动)
 * 消抖时间: 20ms (连续2次扫描一致判定有效)
 *
 * 引脚映射:
 *   KEY1=PA0 (灯光切换),  KEY2=PA1 (风扇切换)
 *   KEY3=PA2 (插座切换),  KEY4=PA3 (备用切换)
 *   KEY5=PA8 (全部全开),  KEY6=PA4 (全部全关)
 *
 * 修正说明: KEY6 从原方案的 PA9 改至 PA4, 避免与 USART1_TX 冲突
 */

#ifndef __KEY_H
#define __KEY_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ 头文件包含 ============================ */
#include "main.h"

/* ============================ 按键状态枚举 ============================ */
typedef enum {
    KEY_NONE    = 0x00,                  /* 无按键                    */
    KEY_PRESS   = 0x01,                  /* 按键按下 (单次触发)       */
    KEY_RELEASE = 0x02,                  /* 按键释放                  */
    KEY_HOLD    = 0x03                   /* 按键长按                  */
} Key_State_t;

/* ============================ 按键索引枚举 ============================ */
typedef enum {
    KEY_IDX_LIGHT   = 0,                /* KEY1: 灯光切换            */
    KEY_IDX_FAN     = 1,                /* KEY2: 风扇切换            */
    KEY_IDX_SOCKET  = 2,                /* KEY3: 插座切换            */
    KEY_IDX_SPARE   = 3,                /* KEY4: 备用切换            */
    KEY_IDX_ALL_ON  = 4,                /* KEY5: 全部全开            */
    KEY_IDX_ALL_OFF = 5                 /* KEY6: 全部全关            */
} Key_Index_t;

/* ============================ 函数原型 ============================ */

/**
 * @brief  初始化按键 GPIO
 * @note   配置 PA0~PA3, PA4, PA8 为上拉输入模式
 *         内部上拉电阻确保未按下时读取为高电平
 */
void Key_Init(void);

/**
 * @brief  按键扫描函数 (由 TIM2 中断周期性调用)
 * @note   每 10ms 调用一次, 采用状态机实现消抖:
 *         状态0: 检测按键按下 → 状态1
 *         状态1: 确认按键按下 → 状态2 (消抖通过)
 *         状态2: 等待按键释放 → 状态0
 *         检测到有效按键后置位 key_flag[]
 */
void Key_Scan(void);

/**
 * @brief  按键处理函数 (由主循环调用)
 * @note   读取 key_flag[] 并执行对应的家电控制逻辑
 *         KEY1~KEY4: 切换对应继电器状态
 *         KEY5:      全部继电器吸合
 *         KEY6:      全部继电器断开
 */
void Key_Process(void);

/**
 * @brief  获取指定按键的当前物理电平
 * @param  key_idx 按键索引 (0~5)
 * @return uint8_t  0=按下 (低电平), 1=未按下 (高电平)
 * @note   按键接 GND, 按下时引脚为低电平
 */
uint8_t Key_Get_Raw_State(uint8_t key_idx);

/**
 * @brief  检查指定按键是否被触发
 * @param  key_idx 按键索引
 * @return uint8_t  1=已触发, 0=未触发
 */
uint8_t Key_Is_Triggered(uint8_t key_idx);

/**
 * @brief  清除指定按键的触发标志
 * @param  key_idx 按键索引
 */
void Key_Clear_Flag(uint8_t key_idx);

#ifdef __cplusplus
}
#endif

#endif /* __KEY_H */
