/**
 * @file    usart.c
 * @brief   USART1 串口通信模块 - 实现
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 通过 USART1 模拟 LD3320 离线语音识别模块:
 *
 * 仿真方案:
 * - 在实际项目中, LD3320 通过 SPI 与 STM32 通信, 识别完成后通过中断通知主控
 * - 在 Wokwi 仿真中无法模拟真实麦克风, 因此采用以下替代方案:
 *   1. USART1 串口接收中断监听虚拟串口
 *   2. 在 Wokwi Serial Monitor 中发送十六进制 ID (0x01~0x0A)
 *   3. 接收中断触发后, 调用与实际硬件完全一致的 Process_ASR_Result() 函数
 *
 * 优势:
 * - 主控的控制逻辑代码与真实硬件完全一致
 * - 仅需替换语音识别数据的输入方式
 * - 方便调试和演示, 可精确控制每次识别的 ID
 *
 * 串口参数: 9600-8-N-1
 * 引脚: TX=PA9, RX=PA10
 *
 * 知识点覆盖: USART 串口通信、串口接收中断、HAL 回调机制
 */

/* ============================ 头文件包含 ============================ */
#include "usart.h"

/* ============================ 外部变量引用 ============================ */
extern char     asr_result_str[ASR_STR_MAX_LEN];
extern uint8_t  uart_rx_buf[USART_RX_BUF_SIZE];

/* ============================ 静态变量 ============================ */
static uint8_t uart_tx_busy = 0;  /* 发送忙标志 */

/* ============================ 函数实现 ============================ */

/**
 * @brief  初始化 USART1 外设
 * @note   配置参数:
 *         - 波特率: 9600 bps
 *         - 数据位: 8 bit
 *         - 停止位: 1 bit
 *         - 校验位: 无
 *         - 流控制: 无
 *         - 模式: 全双工 (TX + RX)
 *
 *         启动首次 HAL_UART_Receive_IT() 进入持续监听状态
 */
void MX_USART1_UART_Init(void)
{
    huart1.Instance          = USART1;
    huart1.Init.BaudRate     = USART_BAUDRATE;       /* 9600 bps */
    huart1.Init.WordLength   = UART_WORDLENGTH_8B;
    huart1.Init.StopBits     = UART_STOPBITS_1;
    huart1.Init.Parity       = UART_PARITY_NONE;
    huart1.Init.Mode         = UART_MODE_TX_RX;      /* 全双工 */
    huart1.Init.HwFlowCtl    = UART_HWCONTROL_NONE;
    huart1.Init.OverSampling = UART_OVERSAMPLING_16;

    if (HAL_UART_Init(&huart1) != HAL_OK)
    {
        System_Error_Handler();
    }

    /* 启动首次中断接收 (之后由回调函数循环启动) */
    HAL_UART_Receive_IT(&huart1, uart_rx_buf, USART_RX_BUF_SIZE);
}

/**
 * @brief  USART1 接收中断回调
 * @param  huart UART 句柄指针
 *
 * @note   LD3320 语音识别模拟的核心逻辑:
 *         1. 检查接收来源是否为 USART1
 *         2. 获取接收到的字节 (模拟 LD3320 的识别 ID)
 *         3. 验证 ID 有效性 (0x01 ≤ id ≤ 0x0A)
 *         4. 有效 ID: 调用 Process_ASR_Result() 执行家电控制
 *            无效 ID: 显示 "Unknown CMD!" 错误提示
 *         5. 重新启动接收中断, 实现持续监听
 *
 *         此函数中的控制逻辑与真实 LD3320 硬件方案完全一致,
 *         只是在真实方案中 ID 来自 LD3320 的 SPI 寄存器,
 *         仿真中来自 USART1 串口接收
 */
/* HAL_UART_RxCpltCallback() defined in main.c */

/**
 * @brief  通过 USART1 发送数据
 * @param  data 数据缓冲区指针
 * @param  len  数据长度 (字节)
 * @note   阻塞式发送, 使用 HAL_UART_Transmit()
 */
void USART_SendData(uint8_t *data, uint16_t len)
{
    HAL_UART_Transmit(&huart1, data, len, 1000);
}

/**
 * @brief  通过 USART1 发送字符串
 * @param  str 要发送的字符串
 * @note   用于调试输出, 在 Wokwi Serial Monitor 中可查看
 */
void USART_SendString(const char *str)
{
    uint16_t len = 0;

    /* 计算字符串长度 */
    while (str[len] != '\0')
    {
        len++;
    }

    if (len > 0)
    {
        HAL_UART_Transmit(&huart1, (uint8_t *)str, len, 1000);
    }
}

/**
 * @brief  USART1 发送完成回调
 * @param  huart UART 句柄指针
 */
void HAL_UART_TxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uart_tx_busy = 0;  /* 清除发送忙标志 */
    }
}

/**
 * @brief  USART1 错误回调
 * @param  huart UART 句柄指针
 * @note   发生接收错误时, 重新启动接收中断
 */
void HAL_UART_ErrorCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        /* 发生错误时重新启动接收 */
        HAL_UART_Receive_IT(&huart1, uart_rx_buf, USART_RX_BUF_SIZE);
    }
}

/* ============================ printf 重定向 (可选) ============================ */
/**
 * @brief  重定向 printf 到 USART1 (用于调试输出)
 * @param  ch  要发送的字符
 * @param  f   文件指针 (未使用)
 * @return int 发送的字符
 * @note   取消注释以下代码可启用 printf 重定向
 *         需在 Keil 中勾选 "Use MicroLIB"
 */
/*
#if defined(__GNUC__) || defined(__GNUG__)
int _write(int file, char *ptr, int len)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)ptr, len, 1000);
    return len;
}
#else
int fputc(int ch, FILE *f)
{
    HAL_UART_Transmit(&huart1, (uint8_t *)&ch, 1, 1000);
    return ch;
}
#endif
*/
