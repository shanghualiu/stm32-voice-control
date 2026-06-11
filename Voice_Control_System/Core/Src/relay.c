/**
 * @file    relay.c
 * @brief   4路继电器驱动模块 - 实现
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 继电器控制逻辑:
 * - 硬件采用光耦隔离 + PNP 三极管驱动方案
 * - 低电平触发: GPIO 输出 0 → 光耦导通 → 三极管导通 → 继电器吸合
 * - 高电平断开: GPIO 输出 1 → 光耦截止 → 三极管截止 → 继电器断开
 * - 上电初始状态: 所有继电器断开 (GPIO 输出高电平)
 *
 * 知识点覆盖: GPIO 输出控制、外设驱动开发
 */

/* ============================ 头文件包含 ============================ */
#include "relay.h"

/* ============================ 静态变量 ============================ */
static uint8_t relay_state[4] = {RELAY_OFF, RELAY_OFF, RELAY_OFF, RELAY_OFF};

/* ============================ 函数实现 ============================ */

/**
 * @brief  初始化继电器控制 GPIO
 * @note   配置 PB12~PB15 为推挽输出, 初始高电平 (继电器断开)
 *         由于 MX_GPIO_Init() 已统一初始化, 此处仅设置初始状态
 */
void Relay_Init(void)
{
    /* GPIO 初始化已在 MX_GPIO_Init() 中完成 */
    /* 确保所有继电器初始状态为断开 (输出高电平) */
    HAL_GPIO_WritePin(RELAY1_PORT, RELAY1_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RELAY2_PORT, RELAY2_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RELAY3_PORT, RELAY3_PIN, GPIO_PIN_SET);
    HAL_GPIO_WritePin(RELAY4_PORT, RELAY4_PIN, GPIO_PIN_SET);

    /* 初始化内部状态数组 */
    for (uint8_t i = 0; i < 4; i++)
    {
        relay_state[i] = RELAY_OFF;
    }
}

/**
 * @brief  控制单路继电器通断
 * @param  ch    通道号 (1~4)
 * @param  state 目标状态: 1=吸合(低电平), 0=断开(高电平)
 * @note   带参数合法性检查, 无效通道号直接返回
 *         继电器为低电平触发, 因此:
 *         - RELAY_ON  → 写 GPIO_PIN_RESET (0) → 吸合
 *         - RELAY_OFF → 写 GPIO_PIN_SET   (1) → 断开
 */
void Relay_Ctrl(uint8_t ch, uint8_t state)
{
    GPIO_TypeDef   *port;
    uint16_t        pin;
    GPIO_PinState   hal_state;

    /* ---- 参数合法性检查 ---- */
    if (ch < 1 || ch > 4)
    {
        return;  /* 无效通道, 直接返回 */
    }

    /* ---- 根据通道号选择 GPIO 端口和引脚 ---- */
    switch (ch)
    {
        case 1:
            port = RELAY1_PORT;
            pin  = RELAY1_PIN;
            break;
        case 2:
            port = RELAY2_PORT;
            pin  = RELAY2_PIN;
            break;
        case 3:
            port = RELAY3_PORT;
            pin  = RELAY3_PIN;
            break;
        case 4:
            port = RELAY4_PORT;
            pin  = RELAY4_PIN;
            break;
        default:
            return;
    }

    /* ---- 状态映射: 1→低电平(吸合), 0→高电平(断开) ---- */
    hal_state = (state == RELAY_ON) ? GPIO_PIN_RESET : GPIO_PIN_SET;

    /* ---- 执行 GPIO 写入 ---- */
    HAL_GPIO_WritePin(port, pin, hal_state);

    /* ---- 更新内部状态记录 ---- */
    relay_state[ch - 1] = state;
}

/**
 * @brief  获取单路继电器当前状态
 * @param  ch  通道号 (1~4)
 * @return uint8_t 1=吸合, 0=断开; 无效通道返回 0
 */
uint8_t Relay_Get_Status(uint8_t ch)
{
    if (ch < 1 || ch > 4)
    {
        return 0;
    }
    return relay_state[ch - 1];
}

/**
 * @brief  全部继电器吸合 (一键全开)
 * @note   依次控制 4 路继电器吸合
 *         预留 5ms 间隔避免同时吸合造成电源冲击
 */
void Relay_All_On(void)
{
    for (uint8_t i = 1; i <= 4; i++)
    {
        Relay_Ctrl(i, RELAY_ON);
        HAL_Delay(5);  /* 5ms 间隔, 减小电源冲击 */
    }
}

/**
 * @brief  全部继电器断开 (一键全关)
 * @note   依次控制 4 路继电器断开
 */
void Relay_All_Off(void)
{
    for (uint8_t i = 1; i <= 4; i++)
    {
        Relay_Ctrl(i, RELAY_OFF);
        HAL_Delay(5);
    }
}

/**
 * @brief  获取所有继电器状态的位掩码
 * @return uint8_t bit0=CH1, bit1=CH2, bit2=CH3, bit3=CH4
 */
uint8_t Relay_Get_All_Status(void)
{
    uint8_t mask = 0;

    for (uint8_t i = 0; i < 4; i++)
    {
        if (relay_state[i] == RELAY_ON)
        {
            mask |= (uint8_t)(1 << i);
        }
    }

    return mask;
}
