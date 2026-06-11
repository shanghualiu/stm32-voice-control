/**
 * @file    usart.h
 * @brief   USART1 串口通信模块 - 头文件
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 通过 USART1 模拟 LD3320 语音识别模块的识别结果输出:
 * - 接收: 串口中断方式接收 1 字节十六进制 ID
 * - 处理: 有效 ID (0x01~0x0A) 直接调用 Process_ASR_Result()
 * - 发送: 可选的调试信息输出 (printf 重定向)
 *
 * 串口参数: 9600bps, 8数据位, 1停止位, 无校验
 * 引脚: TX=PA9, RX=PA10
 *
 * 知识点覆盖: USART 串口通信、串口接收中断、HAL 回调机制
 */

#ifndef __USART_H
#define __USART_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ 头文件包含 ============================ */
#include "main.h"

/* ============================ 语音指令 ID 宏定义 ============================ */
/* 单路控制指令 */
#define ASR_ID_LIGHT_ON     0x01         /* 打开灯光                  */
#define ASR_ID_LIGHT_OFF    0x02         /* 关闭灯光                  */
#define ASR_ID_FAN_ON       0x03         /* 打开风扇                  */
#define ASR_ID_FAN_OFF      0x04         /* 关闭风扇                  */
#define ASR_ID_SOCKET_ON    0x05         /* 打开插座                  */
#define ASR_ID_SOCKET_OFF   0x06         /* 关闭插座                  */
#define ASR_ID_SPARE_ON     0x07         /* 打开备用                  */
#define ASR_ID_SPARE_OFF    0x08         /* 关闭备用                  */

/* 全局控制指令 */
#define ASR_ID_ALL_ON       0x09         /* 全部打开                  */
#define ASR_ID_ALL_OFF      0x0A         /* 全部关闭                  */

/* 有效指令范围 */
#define ASR_ID_MIN          0x01         /* 最小有效 ID               */
#define ASR_ID_MAX          0x0A         /* 最大有效 ID               */

/* ============================ 函数原型 ============================ */

/**
 * @brief  初始化 USART1 外设
 * @note   配置: 9600-8-N-1, 使能接收中断 (RXNE)
 *         启动首次 HAL_UART_Receive_IT() 等待接收
 */
void MX_USART1_UART_Init(void);

/**
 * @brief  USART1 串口接收中断回调
 * @param  huart UART 句柄指针
 * @note   由 HAL 库在接收完成后自动调用
 *         1. 读取接收到的字节 (语音指令 ID)
 *         2. 验证 ID 范围 (0x01~0x0A)
 *         3. 有效则调用 Process_ASR_Result()
 *         4. 重新启动接收中断 (循环接收)
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart);

/**
 * @brief  通过 USART1 发送调试信息
 * @param  data 数据缓冲区指针
 * @param  len  发送长度 (字节)
 * @note   阻塞式发送, 用于调试输出
 */
void USART_SendData(uint8_t *data, uint16_t len);

/**
 * @brief  通过 USART1 发送字符串
 * @param  str 要发送的字符串 (以 '\0' 结尾)
 */
void USART_SendString(const char *str);

/**
 * @brief  USART1 发送完成回调
 * @param  huart UART 句柄指针
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart);

#ifdef __cplusplus
}
#endif

#endif /* __USART_H */
