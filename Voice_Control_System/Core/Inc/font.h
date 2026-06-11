/**
 * @file    font.h
 * @brief   OLED 显示字库 - 头文件
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 提供 ASCII 字符点阵字库数据:
 * - 6x8 字体: 95 个可打印 ASCII 字符 (0x20~0x7E), 每字符 6 字节
 * - 8x16 字体: 95 个可打印 ASCII 字符 (0x20~0x7E), 每字符 16 字节
 *
 * 字库数据存储于 font.c, 使用 const 关键字存放于 Flash (只读段)
 */

#ifndef __FONT_H
#define __FONT_H

#ifdef __cplusplus
extern "C" {
#endif

/* ============================ 头文件包含 ============================ */
#include "main.h"

/* ============================ 字库参数 ============================ */
#define FONT6X8_CHAR_COUNT  95           /* 6x8 字体字符数 (0x20~0x7E) */
#define FONT6X8_BYTES       6            /* 6x8 字体每字符字节数       */
#define FONT8X16_CHAR_COUNT 95           /* 8x16 字体字符数            */
#define FONT8X16_BYTES      16           /* 8x16 字体每字符字节数      */

/* ============================ ASCII 码范围 ============================ */
#define ASCII_PRINTABLE_MIN 0x20         /* 可打印字符起始 ' '        */
#define ASCII_PRINTABLE_MAX 0x7E         /* 可打印字符结束 '~'        */

/* ============================ 外部字库声明 ============================ */

/** @brief 6x8 ASCII 点阵字库 (95字符 × 6字节) */
extern const uint8_t Font_6x8[FONT6X8_CHAR_COUNT][FONT6X8_BYTES];

/** @brief 8x16 ASCII 点阵字库 (95字符 × 16字节) */
extern const uint8_t Font_8x16[FONT8X16_CHAR_COUNT][FONT8X16_BYTES];

/* ============================ 函数原型 ============================ */

/**
 * @brief  获取字符在 6x8 字库中的点阵数据指针
 * @param  ch ASCII 字符
 * @return const uint8_t* 指向该字符的 6 字节点阵数据, 非法字符返回空格
 */
const uint8_t* Font_Get_6x8(uint8_t ch);

/**
 * @brief  获取字符在 8x16 字库中的点阵数据指针
 * @param  ch ASCII 字符
 * @return const uint8_t* 指向该字符的 16 字节点阵数据, 非法字符返回空格
 */
const uint8_t* Font_Get_8x16(uint8_t ch);

#ifdef __cplusplus
}
#endif

#endif /* __FONT_H */
