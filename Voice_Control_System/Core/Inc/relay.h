/**
 * @file    relay.h
 * @brief   4路继电器驱动模块 - 头文件
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 提供4路光耦隔离继电器的独立控制和全局控制接口。
 * 继电器采用低电平触发方式 (光耦隔离驱动)。
 * GPIO 引脚: PB12(灯光) / PB13(风扇) / PB14(插座) / PB15(备用)
 */

#ifndef __RELAY_H
#define __RELAY_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ 头文件包含 ============================ */
#include "main.h"

/* ============================ 继电器动作宏 ============================ */
#define RELAY_ON          1               /* 继电器吸合 (设备通电)     */
#define RELAY_OFF         0               /* 继电器断开 (设备断电)     */

/* ============================ 函数原型 ============================ */

/**
 * @brief  初始化继电器控制GPIO
 * @note   配置 PB12~PB15 为推挽输出, 初始状态为断开 (高电平)
 *         继电器为低电平触发, 高电平时继电器断开, 低电平时吸合
 */
void Relay_Init(void);

/**
 * @brief  控制单路继电器通断
 * @param  ch   继电器通道号 (1~4)
 * @param  state 目标状态: RELAY_ON(1)=吸合, RELAY_OFF(0)=断开
 * @note   继电器为低电平触发: 写0吸合, 写1断开
 */
void Relay_Ctrl(uint8_t ch, uint8_t state);

/**
 * @brief  获取单路继电器当前状态
 * @param  ch  继电器通道号 (1~4)
 * @return uint8_t 当前状态: 1=吸合, 0=断开
 */
uint8_t Relay_Get_Status(uint8_t ch);

/**
 * @brief  全部继电器吸合 (一键全开)
 * @note   调用 Relay_Ctrl() 逐一控制4路继电器吸合
 */
void Relay_All_On(void);

/**
 * @brief  全部继电器断开 (一键全关)
 * @note   调用 Relay_Ctrl() 逐一控制4路继电器断开
 */
void Relay_All_Off(void);

/**
 * @brief  获取所有继电器状态的位掩码
 * @return uint8_t 低4位对应4路继电器状态 (bit0=CH1, ..., bit3=CH4)
 */
uint8_t Relay_Get_All_Status(void);

#ifdef __cplusplus
}
#endif

#endif /* __RELAY_H */
