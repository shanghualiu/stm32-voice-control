/**
 * @file    main.c
 * @brief   基于离线语音识别的家电控制系统 - 主程序
 * @author  嵌入式课程设计
 * @date    2026-06-11
 * @version V1.0 (Wokwi 纯仿真版)
 *
 * 功能概述:
 *   1. 系统初始化: 时钟(72MHz)、GPIO、I2C(OLED)、USART1(模拟LD3320)、TIM2(按键消抖)
 *   2. 按键处理: 6路独立按键控制家电开关
 *   3. 语音识别: USART1 串口接收中断模拟 LD3320 识别结果
 *   4. 继电器控制: 4路光耦继电器驱动灯光/风扇/插座/备用设备
 *   5. OLED 显示: 实时显示识别结果和4路设备状态
 *
 * 知识点覆盖:
 *   - GPIO 输入/输出 (按键输入、继电器输出)
 *   - USART 串口通信 + 接收中断 (LD3320 模拟)
 *   - I2C 总线通信 (SSD1306 OLED 驱动)
 *   - 定时器中断 TIM2 (10ms 按键扫描消抖)
 *   - 外设驱动开发 (继电器、OLED、按键模块化驱动)
 *
 * 修正记录:
 *   V1.0: 将 KEY6 引脚从 PA9 改至 PA4, 解决与 USART1_TX 冲突
 */

/* ============================ 头文件包含 ============================ */
#include "main.h"
#include "relay.h"
#include "oled.h"
#include "key.h"
#include "tim.h"
#include "usart.h"

/* ============================ 外设句柄定义 ============================ */
I2C_HandleTypeDef    hi2c1;       /* I2C1 句柄: OLED 通信         */
TIM_HandleTypeDef    htim2;       /* TIM2 句柄: 按键消抖定时器    */
UART_HandleTypeDef   huart1;      /* USART1 句柄: LD3320 模拟通信  */

/* ============================ 全局变量定义 ============================ */
uint8_t  key_buf[KEY_COUNT]       = {0};  /* 按键状态缓冲区          */
uint8_t  key_flag[KEY_COUNT]      = {0};  /* 按键触发标志            */
uint8_t  relay_status[4]          = {0};  /* 继电器当前状态          */
char     asr_result_str[ASR_STR_MAX_LEN] = "Waiting...";  /* 识别结果 */
uint8_t  uart_rx_buf[USART_RX_BUF_SIZE]  = {0};  /* UART接收缓冲区   */
uint8_t  sys_state                = SYS_STATE_INIT; /* 系统状态       */
volatile uint32_t g_ms_counter    = 0;    /* 全局毫秒计数器           */

/* ============================ 主函数 ============================ */
/**
 * @brief  系统主函数
 * @note   初始化所有外设后进入主循环:
 *         按键处理 → 状态更新 → OLED刷新 → 100ms延时
 */
int main(void)
{
    /* ---- 第1步: HAL 库初始化 ---- */
    HAL_Init();

    /* ---- 第2步: 系统时钟配置 (72MHz) ---- */
    SystemClock_Config();

    /* ---- 第3步: 外设初始化 ---- */
    MX_GPIO_Init();              /* GPIO: 按键输入 + 继电器输出     */
    MX_I2C1_Init();              /* I2C1: OLED 显示通信             */
    MX_USART1_UART_Init();       /* USART1: 模拟 LD3320 语音识别    */
    TIM2_Init();                 /* TIM2: 10ms 按键扫描消抖         */

    /* ---- 第4步: 模块驱动初始化 ---- */
    Relay_Init();                /* 继电器 GPIO 初始化 (全部断开)    */
    Key_Init();                  /* 按键 GPIO 初始化 (上拉输入)      */
    OLED_Init();                 /* SSD1306 OLED 初始化序列          */

    /* ---- 第5步: 显示启动画面 ---- */
    OLED_Show_BootScreen();
    HAL_Delay(1500);             /* 启动画面显示 1.5 秒              */

    /* ---- 第6步: 进入就绪状态 ---- */
    sys_state = SYS_STATE_READY;
    strcpy(asr_result_str, "System Ready");
    OLED_Update_Main(asr_result_str, relay_status);

    /* ---- 第7步: 主循环 ---- */
    while (1)
    {
        /* 7.1 按键扫描与处理 */
        Key_Process();

        /* 7.2 更新继电器状态数组 */
        for (uint8_t i = 0; i < 4; i++)
        {
            relay_status[i] = Relay_Get_Status((uint8_t)(i + 1));
        }

        /* 7.3 OLED 显示刷新 */
        OLED_Update_Main(asr_result_str, relay_status);

        /* 7.4 100ms 主循环周期 */
        HAL_Delay(100);
    }
}

/* ============================ 系统时钟配置 ============================ */
/**
 * @brief  配置系统时钟为 72MHz
 * @note   时钟源: HSE (8MHz 外部晶振)
 *         路径: HSE → PLL (×9) → SYSCLK (72MHz)
 *         APB1 = 36MHz, APB2 = 72MHz
 *         Flash 等待周期: 2 (72MHz 要求)
 */
void SystemClock_Config(void)
{
    RCC_OscInitTypeDef RCC_OscInitStruct = {0};
    RCC_ClkInitTypeDef RCC_ClkInitStruct = {0};

    /* ---- 使能 HSE 振荡器, 配置 PLL ---- */
    RCC_OscInitStruct.OscillatorType = RCC_OSCILLATORTYPE_HSE;
    RCC_OscInitStruct.HSEState       = RCC_HSE_ON;
    RCC_OscInitStruct.HSEPredivValue = RCC_HSE_PREDIV_DIV1;
    RCC_OscInitStruct.PLL.PLLState   = RCC_PLL_ON;
    RCC_OscInitStruct.PLL.PLLSource  = RCC_PLLSOURCE_HSE;
    RCC_OscInitStruct.PLL.PLLMUL     = RCC_PLL_MUL9;  /* 8MHz × 9 = 72MHz */
    if (HAL_RCC_OscConfig(&RCC_OscInitStruct) != HAL_OK)
    {
        System_Error_Handler();
    }

    /* ---- 配置系统时钟、AHB、APB1、APB2 ---- */
    RCC_ClkInitStruct.ClockType      = RCC_CLOCKTYPE_HCLK  |
                                       RCC_CLOCKTYPE_SYSCLK |
                                       RCC_CLOCKTYPE_PCLK1  |
                                       RCC_CLOCKTYPE_PCLK2;
    RCC_ClkInitStruct.SYSCLKSource   = RCC_SYSCLKSOURCE_PLLCLK;  /* PLL 输出 */
    RCC_ClkInitStruct.AHBCLKDivider  = RCC_SYSCLK_DIV1;   /* HCLK  = 72MHz */
    RCC_ClkInitStruct.APB1CLKDivider = RCC_HCLK_DIV2;     /* APB1  = 36MHz */
    RCC_ClkInitStruct.APB2CLKDivider = RCC_HCLK_DIV1;     /* APB2  = 72MHz */
    if (HAL_RCC_ClockConfig(&RCC_ClkInitStruct, FLASH_LATENCY_2) != HAL_OK)
    {
        System_Error_Handler();
    }
}

/* ============================ GPIO 初始化 ============================ */
/**
 * @brief  初始化所有 GPIO 引脚
 * @note   继电器输出: PB12~PB15 (推挽输出, 初始高电平=断开)
 *         按键输入:    PA0~PA3, PA4, PA8 (上拉输入)
 */
void MX_GPIO_Init(void)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    /* ---- 使能 GPIO 时钟 ---- */
    __HAL_RCC_GPIOA_CLK_ENABLE();
    __HAL_RCC_GPIOB_CLK_ENABLE();

    /* ---- 继电器输出引脚: PB12~PB15 ---- */
    GPIO_InitStruct.Pin   = GPIO_PIN_12 | GPIO_PIN_13 |
                           GPIO_PIN_14 | GPIO_PIN_15;
    GPIO_InitStruct.Mode  = GPIO_MODE_OUTPUT_PP;      /* 推挽输出 */
    GPIO_InitStruct.Pull  = GPIO_NOPULL;
    GPIO_InitStruct.Speed = GPIO_SPEED_FREQ_LOW;      /* 低速 (继电器) */
    HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

    /* 初始状态: 全部输出高电平 (继电器断开) */
    HAL_GPIO_WritePin(GPIOB, GPIO_PIN_12 | GPIO_PIN_13 |
                             GPIO_PIN_14 | GPIO_PIN_15, GPIO_PIN_SET);

    /* ---- 按键输入引脚: PA0~PA3, PA4, PA8 ---- */
    GPIO_InitStruct.Pin   = GPIO_PIN_0 | GPIO_PIN_1 |
                           GPIO_PIN_2 | GPIO_PIN_3 |
                           GPIO_PIN_4 | GPIO_PIN_8;
    GPIO_InitStruct.Mode  = GPIO_MODE_INPUT;           /* 输入模式 */
    GPIO_InitStruct.Pull  = GPIO_PULLUP;               /* 内部上拉 */
    HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);
}

/* ============================ I2C1 初始化 ============================ */
/**
 * @brief  初始化 I2C1 外设 (用于 OLED 通信)
 * @note   引脚: SCL=PB6, SDA=PB7
 *         速率: 400kHz (Fast Mode)
 *         时钟源: APB1 = 36MHz
 */
void MX_I2C1_Init(void)
{
    hi2c1.Instance             = I2C1;
    hi2c1.Init.ClockSpeed      = 400000;            /* 400kHz Fast Mode */
    hi2c1.Init.DutyCycle       = I2C_DUTYCYCLE_2;   /* Fast Mode 占空比 */
    hi2c1.Init.OwnAddress1     = 0x00;              /* 本机地址 (从机模式用) */
    hi2c1.Init.AddressingMode  = I2C_ADDRESSINGMODE_7BIT;
    hi2c1.Init.DualAddressMode = I2C_DUALADDRESS_DISABLE;
    hi2c1.Init.GeneralCallMode = I2C_GENERALCALL_DISABLE;
    hi2c1.Init.NoStretchMode   = I2C_NOSTRETCH_DISABLE;

    if (HAL_I2C_Init(&hi2c1) != HAL_OK)
    {
        System_Error_Handler();
    }
}

/* MX_USART1_UART_Init() defined in usart.c */
/* MX_TIM2_Init() defined in tim.c as TIM2_Init() */

/* ============================ 语音识别处理函数 ============================ */
/**
 * @brief  处理语音/串口识别结果并执行对应操作
 * @param  result_id 识别结果 ID (0x01~0x0A), 对应语音指令
 * @note   本函数是语音控制的核心, 接收来自两个来源的指令:
 *         1. USART1 串口接收中断 (模拟 LD3320)
 *         2. 按键处理函数 (模拟手动触发)
 *
 *         指令映射表:
 *         0x01→打开灯光  0x02→关闭灯光  0x03→打开风扇  0x04→关闭风扇
 *         0x05→打开插座  0x06→关闭插座  0x07→打开备用  0x08→关闭备用
 *         0x09→全部打开  0x0A→全部关闭
 */
void Process_ASR_Result(uint8_t result_id)
{
    switch (result_id)
    {
        /* ---- 单路控制指令 ---- */
        case 0x01:
            strcpy(asr_result_str, "Light: ON");
            Relay_Ctrl(RELAY_CH_LIGHT, RELAY_ON);
            break;

        case 0x02:
            strcpy(asr_result_str, "Light: OFF");
            Relay_Ctrl(RELAY_CH_LIGHT, RELAY_OFF);
            break;

        case 0x03:
            strcpy(asr_result_str, "Fan: ON");
            Relay_Ctrl(RELAY_CH_FAN, RELAY_ON);
            break;

        case 0x04:
            strcpy(asr_result_str, "Fan: OFF");
            Relay_Ctrl(RELAY_CH_FAN, RELAY_OFF);
            break;

        case 0x05:
            strcpy(asr_result_str, "Socket: ON");
            Relay_Ctrl(RELAY_CH_SOCKET, RELAY_ON);
            break;

        case 0x06:
            strcpy(asr_result_str, "Socket: OFF");
            Relay_Ctrl(RELAY_CH_SOCKET, RELAY_OFF);
            break;

        case 0x07:
            strcpy(asr_result_str, "Spare: ON");
            Relay_Ctrl(RELAY_CH_SPARE, RELAY_ON);
            break;

        case 0x08:
            strcpy(asr_result_str, "Spare: OFF");
            Relay_Ctrl(RELAY_CH_SPARE, RELAY_OFF);
            break;

        /* ---- 全局控制指令 ---- */
        case 0x09:
            strcpy(asr_result_str, "ALL: ON");
            Relay_All_On();
            break;

        case 0x0A:
            strcpy(asr_result_str, "ALL: OFF");
            Relay_All_Off();
            break;

        /* ---- 无效指令 ---- */
        default:
            strcpy(asr_result_str, "Unknown CMD!");
            break;
    }
}

/* ============================ USART1 接收中断回调 ============================ */
/**
 * @brief  USART1 接收完成回调函数
 * @param  huart UART 句柄指针
 * @note   在串口接收中断中自动调用 (由 HAL_UART_IRQHandler 触发)
 *         1. 检查是否为 USART1 实例
 *         2. 验证接收到的 ID 是否在有效范围 (0x01~0x0A)
 *         3. 有效则调用 Process_ASR_Result() 处理
 *         4. 无效则显示 "Unknown CMD!"
 *         5. 重新启动接收中断, 实现持续监听
 */
void HAL_UART_RxCpltCallback(UART_HandleTypeDef *huart)
{
    if (huart->Instance == USART1)
    {
        uint8_t asr_id = uart_rx_buf[0];          /* 读取接收到的字节 */

        if (asr_id >= ASR_ID_MIN && asr_id <= ASR_ID_MAX)
        {
            Process_ASR_Result(asr_id);            /* 有效指令: 执行控制 */
        }
        else
        {
            strcpy(asr_result_str, "Unknown CMD!"); /* 无效指令: 提示错误 */
        }

        /* 重新启动中断接收 (实现循环接收) */
        HAL_UART_Receive_IT(&huart1, uart_rx_buf, USART_RX_BUF_SIZE);
    }
}

/* ============================ 定时器中断回调 ============================ */
/**
 * @brief  TIM2 周期中断回调
 * @param  htim 定时器句柄指针
 * @note   每 10ms 触发一次, 用于:
 *         1. 更新系统毫秒计数器
 *         2. 执行按键扫描与消抖
 */
void HAL_TIM_PeriodElapsedCallback(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        g_ms_counter += 10;          /* 累加 10ms */
        Key_Scan();                  /* 按键扫描 */
    }
}

/* ============================ I2C 初始化回调 ============================ */
/**
 * @brief  I2C 外设 MSP 初始化回调 (引脚配置)
 * @param  hi2c I2C 句柄指针
 * @note   由 HAL_I2C_Init() 自动调用, 配置 I2C 对应的 GPIO 引脚
 *         引脚: SCL=PB6 (复用开漏), SDA=PB7 (复用开漏)
 */
void HAL_I2C_MspInit(I2C_HandleTypeDef *hi2c)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (hi2c->Instance == I2C1)
    {
        /* ---- 使能 I2C1 和 GPIOB 时钟 ---- */
        __HAL_RCC_I2C1_CLK_ENABLE();
        __HAL_RCC_GPIOB_CLK_ENABLE();

        /* ---- 配置 PB6 (SCL) 和 PB7 (SDA) ---- */
        GPIO_InitStruct.Pin       = GPIO_PIN_6 | GPIO_PIN_7;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_OD;          /* 复用开漏 */
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;     /* 高速 */
        HAL_GPIO_Init(GPIOB, &GPIO_InitStruct);

        /* ---- 配置 NVIC 中断 ---- */
        HAL_NVIC_SetPriority(I2C1_EV_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(I2C1_EV_IRQn);
        HAL_NVIC_SetPriority(I2C1_ER_IRQn, 3, 0);
        HAL_NVIC_EnableIRQ(I2C1_ER_IRQn);
    }
}

/* ============================ USART 初始化回调 ============================ */
/**
 * @brief  USART 外设 MSP 初始化回调 (引脚配置)
 * @param  huart UART 句柄指针
 * @note   配置 USART1 对应的 GPIO: TX=PA9 (复用推挽), RX=PA10 (浮空输入)
 */
void HAL_UART_MspInit(UART_HandleTypeDef *huart)
{
    GPIO_InitTypeDef GPIO_InitStruct = {0};

    if (huart->Instance == USART1)
    {
        /* ---- 使能 USART1 和 GPIOA 时钟 ---- */
        __HAL_RCC_USART1_CLK_ENABLE();
        __HAL_RCC_GPIOA_CLK_ENABLE();

        /* ---- 配置 PA9 (TX): 复用推挽输出 ---- */
        GPIO_InitStruct.Pin       = GPIO_PIN_9;
        GPIO_InitStruct.Mode      = GPIO_MODE_AF_PP;          /* 复用推挽 */
        GPIO_InitStruct.Speed     = GPIO_SPEED_FREQ_HIGH;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* ---- 配置 PA10 (RX): 浮空输入 ---- */
        GPIO_InitStruct.Pin       = GPIO_PIN_10;
        GPIO_InitStruct.Mode      = GPIO_MODE_INPUT;          /* 浮空输入 */
        GPIO_InitStruct.Pull      = GPIO_NOPULL;
        HAL_GPIO_Init(GPIOA, &GPIO_InitStruct);

        /* ---- 配置 NVIC 中断 ---- */
        HAL_NVIC_SetPriority(USART1_IRQn, 1, 0);             /* 优先级1 (较高) */
        HAL_NVIC_EnableIRQ(USART1_IRQn);
    }
}

/* ============================ TIM 初始化回调 ============================ */
/**
 * @brief  TIM 外设 MSP 初始化回调
 * @param  htim 定时器句柄指针
 * @note   配置 TIM2 的 NVIC 中断优先级
 */
void HAL_TIM_Base_MspInit(TIM_HandleTypeDef *htim)
{
    if (htim->Instance == TIM2)
    {
        /* ---- 使能 TIM2 时钟 ---- */
        __HAL_RCC_TIM2_CLK_ENABLE();

        /* ---- 配置 NVIC ---- */
        HAL_NVIC_SetPriority(TIM2_IRQn, 2, 0);               /* 优先级2 */
        HAL_NVIC_EnableIRQ(TIM2_IRQn);
    }
}

/* ============================ 系统错误处理 ============================ */
/**
 * @brief  系统错误处理函数
 * @note   发生不可恢复的初始化错误时调用
 *         在 Wokwi 仿真中: 进入无限循环, 可通过调试器查看错误位置
 *         在实物中: 可扩展为 LED 闪烁报警或复位
 */
void System_Error_Handler(void)
{
    /* 关闭全局中断, 进入死循环 */
    __disable_irq();
    sys_state = SYS_STATE_ERROR;

    while (1)
    {
        /* 错误状态: 可根据需要添加 LED 闪烁等指示 */
        /* 在 Wokwi 中, 仿真会在此处停止, 方便调试 */
    }
}

/* ============================ 断言失败处理 ============================ */
/**
 * @brief  HAL 库断言失败回调
 * @param  file 文件名
 * @param  line 行号
 * @note   HAL 库参数检查失败时调用, 用于调试
 */
#ifdef USE_FULL_ASSERT
void assert_failed(uint8_t *file, uint32_t line)
{
    /* 可在此处设置断点或记录日志 */
    while (1)
    {
    }
}
#endif /* USE_FULL_ASSERT */
