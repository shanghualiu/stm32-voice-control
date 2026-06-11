/**
 * @file    oled.h
 * @brief   SSD1306 OLED 显示驱动模块 - 头文件
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 基于 I2C 通信协议驱动 0.96 寸 SSD1306 OLED 显示屏 (128x64 像素)。
 * I2C 地址: 0x3C (7位), 写入地址: 0x78
 * 引脚: SCL=PB6, SDA=PB7
 */

#ifndef __OLED_H
#define __OLED_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ 头文件包含 ============================ */
#include "main.h"

/* ============================ 显示宏定义 ============================ */
#define OLED_CMD          0x00            /* 命令模式标识              */
#define OLED_DATA         0x40            /* 数据模式标识              */

/* 字符显示参数 */
#define FONT_WIDTH_6X8    6               /* 6x8 字体宽度              */
#define FONT_HEIGHT_6X8   8               /* 6x8 字体高度              */
#define FONT_WIDTH_8X16   8               /* 8x16 字体宽度             */
#define FONT_HEIGHT_8X16  16              /* 8x16 字体高度             */

/* 屏幕行号 (基于 8x16 字体) */
#define OLED_LINE_0       0               /* 第0行 Y=0                 */
#define OLED_LINE_1       2               /* 第1行 Y=2 (页)            */
#define OLED_LINE_2       4               /* 第2行 Y=4                 */
#define OLED_LINE_3       6               /* 第3行 Y=6                 */

/* 最大显示字符串长度 */
#define OLED_STR_MAX_LEN  21              /* 128/6 = 21 字符 (6x8字体) */

/* ============================ 函数原型 ============================ */

/**
 * @brief  OLED 初始化
 * @note   执行 SSD1306 上电初始化序列, 清除屏幕内容
 *         配置: 正常显示, 水平寻址模式, 无翻转
 */
void OLED_Init(void);

/**
 * @brief  清除整个 OLED 显示缓冲区
 * @note   将所有 GDDRAM 填充为 0x00, 实现清屏效果
 */
void OLED_Clear(void);

/**
 * @brief  设置光标位置
 * @param  x 列地址 (0~127)
 * @param  y 页地址 (0~7, 对应行 0~63)
 */
void OLED_Set_Pos(uint8_t x, uint8_t y);

/**
 * @brief  在指定位置显示一个 6x8 字符
 * @param  x    列坐标 (0~127)
 * @param  y    页坐标 (0~7)
 * @param  ch   要显示的 ASCII 字符
 */
void OLED_ShowChar_6x8(uint8_t x, uint8_t y, uint8_t ch);

/**
 * @brief  在指定位置显示 6x8 字符串
 * @param  x    起始列坐标
 * @param  y    页坐标
 * @param  str  要显示的字符串 (以 '\0' 结尾)
 */
void OLED_ShowString_6x8(uint8_t x, uint8_t y, const char *str);

/**
 * @brief  在指定位置显示一个 8x16 字符
 * @param  x    列坐标
 * @param  y    页坐标
 * @param  ch   要显示的 ASCII 字符
 */
void OLED_ShowChar_8x16(uint8_t x, uint8_t y, uint8_t ch);

/**
 * @brief  在指定位置显示 8x16 字符串
 * @param  x    起始列坐标
 * @param  y    页坐标
 * @param  str  要显示的字符串
 */
void OLED_ShowString_8x16(uint8_t x, uint8_t y, const char *str);

/**
 * @brief  显示/隐藏 OLED 屏幕
 * @param  on  1=显示, 0=关闭显示 (进入睡眠模式)
 */
void OLED_Display_On(uint8_t on);

/**
 * @brief  更新主界面显示
 * @param  asr_result   语音识别结果字符串 (最长21字符)
 * @param  relay_status 4路继电器状态数组 (0=断开, 1=吸合)
 * @note   显示内容:
 *         Line 0: 系统标题 "=== Smart Home ==="
 *         Line 1: 语音识别结果
 *         Line 2: 设备状态 "1:O 2:X 3:O 4:X"
 *         Line 3: 系统提示 "KEY:Manual  UART:Voice"
 */
void OLED_Update_Main(const char *asr_result, const uint8_t *relay_status);

/**
 * @brief  显示启动画面
 * @note   系统上电时短暂显示项目信息
 */
void OLED_Show_BootScreen(void);

#ifdef __cplusplus
}
#endif

#endif /* __OLED_H */
