/**
 * @file    main.h
 * @brief   基于离线语音识别的家电控制系统 - 主头文件
 * @author  嵌入式课程设计
 * @date    2026-06-11
 * @version V1.0
 *
 * 本文件包含系统全局宏定义、外设句柄声明、全局变量声明及函数原型。
 * 芯片: STM32F103C8T6, 系统时钟: 72MHz (HSE + PLL)
 */

#ifndef __MAIN_H
#define __MAIN_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ 头文件包含 ============================ */
#include "stm32f1xx_hal.h"
#include <string.h>
#include <stdio.h>

/* ============================ 系统配置宏 ============================ */
#define HSE_VALUE          8000000U   /* 外部高速晶振频率 (Hz)      */
#define HSI_VALUE          8000000U   /* 内部高速晶振频率 (Hz)      */
#define SYSTEM_CLOCK      72000000U   /* 系统主频 72MHz             */

/* ============================ 继电器通道定义 ============================ */
#define RELAY_CH_LIGHT     1           /* 通道1: 灯光                */
#define RELAY_CH_FAN       2           /* 通道2: 风扇                */
#define RELAY_CH_SOCKET    3           /* 通道3: 插座                */
#define RELAY_CH_SPARE     4           /* 通道4: 备用设备            */

/* ============================ 继电器控制引脚 ============================ */
#define RELAY1_PORT        GPIOB
#define RELAY1_PIN         GPIO_PIN_12
#define RELAY2_PORT        GPIOB
#define RELAY2_PIN         GPIO_PIN_13
#define RELAY3_PORT        GPIOB
#define RELAY3_PIN         GPIO_PIN_14
#define RELAY4_PORT        GPIOB
#define RELAY4_PIN         GPIO_PIN_15

/* ============================ 按键引脚定义 ============================ */
/* 修正说明: KEY6 从 PA9 改至 PA4, 避免与 USART1_TX 冲突 */
#define KEY1_PORT          GPIOA
#define KEY1_PIN           GPIO_PIN_0
#define KEY2_PORT          GPIOA
#define KEY2_PIN           GPIO_PIN_1
#define KEY3_PORT          GPIOA
#define KEY3_PIN           GPIO_PIN_2
#define KEY4_PORT          GPIOA
#define KEY4_PIN           GPIO_PIN_3
#define KEY5_PORT          GPIOA
#define KEY5_PIN           GPIO_PIN_8     /* 全部全开按键            */
#define KEY6_PORT          GPIOA
#define KEY6_PIN           GPIO_PIN_4     /* 全部全关按键(已修正)    */

/* ============================ 按键数量 ============================ */
#define KEY_COUNT          6

/* ============================ 按键消抖参数 ============================ */
#define KEY_DEBOUNCE_MS    20             /* 消抖时间 (ms)            */
#define KEY_LONG_PRESS_MS  1000           /* 长按判定时间 (ms)        */
#define KEY_SCAN_PERIOD_MS 10             /* 按键扫描周期 (ms)        */

/* ============================ OLED 显示参数 ============================ */
#define OLED_I2C_ADDR      0x78           /* SSD1306 I2C 地址 (7bit:0x3C<<1) */
#define OLED_WIDTH         128            /* OLED 像素宽度             */
#define OLED_HEIGHT        64             /* OLED 像素高度             */
#define OLED_PAGES         8              /* OLED 页数 (每页8像素高)   */
#define OLED_I2C_TIMEOUT   100            /* I2C 通信超时 (ms)         */

/* ============================ 串口通信参数 ============================ */
#define USART_BAUDRATE     9600           /* 串口波特率               */
#define USART_RX_BUF_SIZE  1              /* 接收缓冲区大小            */

/* ============================ 系统状态定义 ============================ */
#define SYS_STATE_INIT     0              /* 系统初始化状态            */
#define SYS_STATE_READY    1              /* 系统就绪状态              */
#define SYS_STATE_RUNNING  2              /* 系统运行状态              */
#define SYS_STATE_ERROR    3              /* 系统错误状态              */

/* ============================ 语音识别结果字符串 ============================ */
#define ASR_STR_MAX_LEN    32             /* 识别结果字符串最大长度    */

/* ============================ 外设句柄外部声明 ============================ */
extern I2C_HandleTypeDef    hi2c1;        /* I2C1 句柄 (OLED)         */
extern TIM_HandleTypeDef    htim2;        /* TIM2 句柄 (按键消抖)     */
extern UART_HandleTypeDef   huart1;       /* USART1 句柄 (LD3320模拟) */

/* ============================ 全局变量外部声明 ============================ */
extern uint8_t  key_buf[KEY_COUNT];        /* 按键状态缓冲区           */
extern uint8_t  key_flag[KEY_COUNT];       /* 按键触发标志             */
extern uint8_t  relay_status[4];           /* 继电器状态数组           */
extern char     asr_result_str[ASR_STR_MAX_LEN]; /* 语音识别结果字符串  */
extern uint8_t  uart_rx_buf[USART_RX_BUF_SIZE]; /* 串口接收缓冲区       */
extern uint8_t  sys_state;                 /* 系统运行状态             */
extern uint32_t sys_tick_count;            /* 系统滴答计数 (ms)        */

/* ============================ 函数原型声明 ============================ */

/* 系统初始化 */
void SystemClock_Config(void);             /* 系统时钟配置 (72MHz)     */
void MX_GPIO_Init(void);                   /* GPIO 初始化              */
void MX_I2C1_Init(void);                   /* I2C1 初始化 (OLED)       */
void MX_USART1_UART_Init(void);            /* USART1 初始化 (LD3320)   */
/* TIM2 初始化声明见 tim.h: TIM2_Init() */

/* 应用函数 */
void Process_ASR_Result(uint8_t result_id); /* 处理语音识别结果         */
void System_Error_Handler(void);            /* 系统错误处理             */

#ifdef __cplusplus
}
#endif

#endif /* __MAIN_H */
