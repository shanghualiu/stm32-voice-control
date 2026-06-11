/**
 * @file    key.c
 * @brief   6键独立按键扫描模块 - 实现
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 按键扫描采用"定时器中断 + 状态机消抖"方案:
 * - 扫描周期: 10ms (由 TIM2 中断驱动)
 * - 消抖方案: 连续 2 次扫描均检测到按下 → 确认有效 (20ms 消抖)
 * - 状态机: 空闲(0) → 按下检测(1) → 消抖确认(2) → 等待释放(3)
 * - 单次触发: 按键按下只触发一次对应操作, 避免重复触发
 *
 * 按键接线: 一端接 STM32 GPIO (内部上拉), 另一端接 GND
 * 按下时 GPIO 读取为低电平 (0), 释放时读取为高电平 (1)
 *
 * 知识点覆盖: GPIO 输入、定时器中断消抖、状态机编程
 */

/* ============================ 头文件包含 ============================ */
#include "key.h"
#include "relay.h"

/* ============================ 按键状态常量 ============================ */
#define KEY_STATE_IDLE    0              /* 空闲: 按键未按下          */
#define KEY_STATE_PRESS   1              /* 按下: 首次检测到低电平    */
#define KEY_STATE_CONFIRM 2              /* 确认: 消抖通过, 等待释放  */

/* ============================ 按键检测阈值 ============================ */
#define KEY_DEBOUNCE_COUNT 2             /* 消抖确认次数 (20ms)        */
#define KEY_LONG_PRESS_COUNT 100         /* 长按阈值 (1s)              */

/* ============================ 静态变量 ============================ */
static uint8_t key_state[KEY_COUNT]  = {0};  /* 按键状态机              */
static uint8_t key_counter[KEY_COUNT] = {0}; /* 消抖/长按计数器        */

/* ============================ 按键 GPIO 映射表 ============================ */
typedef struct {
    GPIO_TypeDef *port;
    uint16_t      pin;
} Key_Pin_t;

static const Key_Pin_t key_pins[KEY_COUNT] = {
    {KEY1_PORT, KEY1_PIN},   /* [0] KEY1: PA0 → 灯光切换 */
    {KEY2_PORT, KEY2_PIN},   /* [1] KEY2: PA1 → 风扇切换 */
    {KEY3_PORT, KEY3_PIN},   /* [2] KEY3: PA2 → 插座切换 */
    {KEY4_PORT, KEY4_PIN},   /* [3] KEY4: PA3 → 备用切换 */
    {KEY5_PORT, KEY5_PIN},   /* [4] KEY5: PA8 → 全部全开 */
    {KEY6_PORT, KEY6_PIN},   /* [5] KEY6: PA4 → 全部全关 (已修正引脚) */
};

/* ============================ 按键功能描述 (用于调试) ============================ */
static const char *key_names[KEY_COUNT] = {
    "KEY1:Light Toggle",
    "KEY2:Fan Toggle",
    "KEY3:Socket Toggle",
    "KEY4:Spare Toggle",
    "KEY5:All ON",
    "KEY6:All OFF"
};

/* ============================ 外部变量引用 ============================ */
extern uint8_t  key_buf[KEY_COUNT];
extern uint8_t  key_flag[KEY_COUNT];
extern char     asr_result_str[ASR_STR_MAX_LEN];
extern uint32_t g_ms_counter;

/* ============================ 函数实现 ============================ */

/**
 * @brief  初始化按键 GPIO
 * @note   GPIO 初始化已在 MX_GPIO_Init() 中完成
 *         此处初始化按键状态机和内部变量
 */
void Key_Init(void)
{
    uint8_t i;

    for (i = 0; i < KEY_COUNT; i++)
    {
        key_state[i]   = KEY_STATE_IDLE;
        key_counter[i] = 0;
        key_buf[i]     = 1;      /* 初始状态: 未按下 (高电平) */
        key_flag[i]    = 0;      /* 初始: 无触发 */
    }
}

/**
 * @brief  获取指定按键的物理电平
 * @param  key_idx 按键索引 (0~5)
 * @return uint8_t 0=按下 (低电平), 1=未按下 (高电平)
 */
uint8_t Key_Get_Raw_State(uint8_t key_idx)
{
    if (key_idx >= KEY_COUNT)
    {
        return 1;  /* 无效索引, 返回未按下 */
    }

    GPIO_PinState pin_state = HAL_GPIO_ReadPin(key_pins[key_idx].port,
                                                key_pins[key_idx].pin);
    return (pin_state == GPIO_PIN_RESET) ? 0 : 1;
}

/**
 * @brief  按键扫描函数 (由 TIM2 中断调用, 每 10ms)
 * @note   状态机流程:
 *         状态0 (IDLE):
 *           检测到低电平 (按下) → 进入状态1, 计数器=0
 *
 *         状态1 (PRESS):
 *           检测到低电平 → 计数器+1
 *             计数器 >= DEBOUNCE_COUNT → 消抖通过, 进入状态2, 置位 key_flag
 *           检测到高电平 → 抖动, 回到状态0
 *
 *         状态2 (CONFIRM):
 *           等待按键释放 (高电平)
 *           检测到高电平 → 回到状态0
 */
void Key_Scan(void)
{
    uint8_t i;
    uint8_t raw_level;

    for (i = 0; i < KEY_COUNT; i++)
    {
        raw_level = Key_Get_Raw_State(i);  /* 读取当前物理电平 */

        switch (key_state[i])
        {
            /* ---- 状态0: 空闲, 等待按下 ---- */
            case KEY_STATE_IDLE:
                if (raw_level == 0)        /* 检测到按下 */
                {
                    key_state[i] = KEY_STATE_PRESS;
                    key_counter[i] = 0;
                }
                break;

            /* ---- 状态1: 按下检测, 消抖中 ---- */
            case KEY_STATE_PRESS:
                if (raw_level == 0)
                {
                    key_counter[i]++;

                    if (key_counter[i] >= KEY_DEBOUNCE_COUNT)
                    {
                        /* 消抖通过: 确认按下 */
                        key_state[i] = KEY_STATE_CONFIRM;
                        key_buf[i] = 0;                  /* 标记为按下 */
                        key_flag[i] = 1;                 /* 置位触发标志 */
                    }
                }
                else
                {
                    /* 抖动: 回到空闲 */
                    key_state[i] = KEY_STATE_IDLE;
                    key_counter[i] = 0;
                }
                break;

            /* ---- 状态2: 已确认, 等待释放 ---- */
            case KEY_STATE_CONFIRM:
                if (raw_level == 1)        /* 检测到释放 */
                {
                    key_state[i] = KEY_STATE_IDLE;
                    key_counter[i] = 0;
                    key_buf[i] = 1;                      /* 标记为释放 */
                }
                /* 仍为低电平时保持当前状态 (防止重复触发) */
                break;

            default:
                key_state[i] = KEY_STATE_IDLE;
                break;
        }
    }
}

/**
 * @brief  按键处理函数 (由主循环调用)
 * @note   读取 key_flag[] 中各按键的触发标志:
 *         KEY1~KEY4: 切换对应继电器状态
 *         KEY5:      全部继电器吸合 (一键全开)
 *         KEY6:      全部继电器断开 (一键全关)
 *
 *         处理完毕后清除对应的触发标志
 */
void Key_Process(void)
{
    uint8_t i;
    uint8_t current_state;

    for (i = 0; i < KEY_COUNT; i++)
    {
        if (key_flag[i] == 1)
        {
            switch (i)
            {
                /* KEY1: 灯光切换 */
                case KEY_IDX_LIGHT:
                    current_state = Relay_Get_Status(RELAY_CH_LIGHT);
                    Relay_Ctrl(RELAY_CH_LIGHT, current_state ? RELAY_OFF : RELAY_ON);
                    strcpy(asr_result_str, current_state ? "Key: Light OFF" : "Key: Light ON");
                    break;

                /* KEY2: 风扇切换 */
                case KEY_IDX_FAN:
                    current_state = Relay_Get_Status(RELAY_CH_FAN);
                    Relay_Ctrl(RELAY_CH_FAN, current_state ? RELAY_OFF : RELAY_ON);
                    strcpy(asr_result_str, current_state ? "Key: Fan OFF" : "Key: Fan ON");
                    break;

                /* KEY3: 插座切换 */
                case KEY_IDX_SOCKET:
                    current_state = Relay_Get_Status(RELAY_CH_SOCKET);
                    Relay_Ctrl(RELAY_CH_SOCKET, current_state ? RELAY_OFF : RELAY_ON);
                    strcpy(asr_result_str, current_state ? "Key: Socket OFF" : "Key: Socket ON");
                    break;

                /* KEY4: 备用切换 */
                case KEY_IDX_SPARE:
                    current_state = Relay_Get_Status(RELAY_CH_SPARE);
                    Relay_Ctrl(RELAY_CH_SPARE, current_state ? RELAY_OFF : RELAY_ON);
                    strcpy(asr_result_str, current_state ? "Key: Spare OFF" : "Key: Spare ON");
                    break;

                /* KEY5: 全部全开 */
                case KEY_IDX_ALL_ON:
                    Relay_All_On();
                    strcpy(asr_result_str, "Key: ALL ON");
                    break;

                /* KEY6: 全部全关 */
                case KEY_IDX_ALL_OFF:
                    Relay_All_Off();
                    strcpy(asr_result_str, "Key: ALL OFF");
                    break;

                default:
                    break;
            }

            /* 清除触发标志 (单次触发) */
            key_flag[i] = 0;
        }
    }
}

/**
 * @brief  检查指定按键是否已触发
 * @param  key_idx 按键索引
 * @return uint8_t 1=已触发, 0=未触发
 */
uint8_t Key_Is_Triggered(uint8_t key_idx)
{
    if (key_idx >= KEY_COUNT)
    {
        return 0;
    }
    return key_flag[key_idx];
}

/**
 * @brief  清除指定按键的触发标志
 * @param  key_idx 按键索引
 */
void Key_Clear_Flag(uint8_t key_idx)
{
    if (key_idx < KEY_COUNT)
    {
        key_flag[key_idx] = 0;
    }
}
