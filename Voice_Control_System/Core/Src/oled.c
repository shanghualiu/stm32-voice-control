/**
 * @file    oled.c
 * @brief   SSD1306 OLED 显示驱动模块 - 实现
 * @author  嵌入式课程设计
 * @date    2026-06-11
 *
 * 基于 I2C 通信驱动 0.96 寸 SSD1306 OLED (128×64 像素)
 * - I2C 地址: 0x3C << 1 = 0x78
 * - 通信协议: 命令模式 (0x00) / 数据模式 (0x40)
 * - 寻址模式: 水平寻址 (Page Addressing Mode)
 *
 * 知识点覆盖: I2C 总线通信、外设驱动开发、显示驱动
 */

/* ============================ 头文件包含 ============================ */
#include "oled.h"
#include "font.h"

/* ============================ 静态函数声明 ============================ */
static void OLED_WriteCmd(uint8_t cmd);
static void OLED_WriteData(uint8_t data);
static void OLED_WriteMultiData(const uint8_t *data, uint16_t len);

/* ============================ 静态变量 ============================ */
static uint8_t OLED_Buffer[OLED_WIDTH];  /* 行缓冲区 (128 字节) */

/* ============================ 低级 I2C 通信函数 ============================ */

/**
 * @brief  向 OLED 发送命令字节
 * @param  cmd 命令字节
 * @note   命令格式: [0x00][cmd]
 */
static void OLED_WriteCmd(uint8_t cmd)
{
    uint8_t buf[2] = {OLED_CMD, cmd};
    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, buf, 2, OLED_I2C_TIMEOUT);
}

/**
 * @brief  向 OLED 发送数据字节
 * @param  data 数据字节
 * @note   数据格式: [0x40][data]
 */
static void OLED_WriteData(uint8_t data)
{
    uint8_t buf[2] = {OLED_DATA, data};
    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, buf, 2, OLED_I2C_TIMEOUT);
}

/**
 * @brief  向 OLED 发送多字节数据
 * @param  data 数据缓冲区指针
 * @param  len  数据长度
 * @note   用于行写入优化: [0x40][data0][data1]...[dataN]
 */
static void OLED_WriteMultiData(const uint8_t *data, uint16_t len)
{
    /* 动态分配临时缓冲区: 控制字节 + 数据 */
    uint8_t buf[OLED_WIDTH + 1];
    buf[0] = OLED_DATA;

    for (uint16_t i = 0; i < len; i++)
    {
        buf[i + 1] = data[i];
    }

    HAL_I2C_Master_Transmit(&hi2c1, OLED_I2C_ADDR, buf, (uint16_t)(len + 1),
                            OLED_I2C_TIMEOUT);
}

/* ============================ OLED 初始化 ============================ */

/**
 * @brief  OLED 初始化序列
 * @note   执行 SSD1306 标准初始化流程:
 *         1. 关闭显示
 *         2. 设置时钟分频和振荡器频率
 *         3. 设置多路复用比 (64)
 *         4. 设置显示偏移 (0)
 *         5. 设置显示起始行 (0)
 *         6. 设置段重映射和 COM 扫描方向
 *         7. 设置 COM 引脚硬件配置
 *         8. 设置对比度
 *         9. 关闭电荷泵 → 全屏点亮 → 开启电荷泵
 *         10. 从睡眠模式唤醒
 *         11. 开启显示
 */
void OLED_Init(void)
{
    /* ---- 等待 OLED 上电稳定 ---- */
    HAL_Delay(100);

    /* ---- SSD1306 初始化命令序列 ---- */
    OLED_WriteCmd(0xAE);  /* 关闭显示 (Display OFF)                    */
    OLED_WriteCmd(0x20);  /* 设置寻址模式                              */
    OLED_WriteCmd(0x00);  /* 水平寻址模式                              */
    OLED_WriteCmd(0x40);  /* 设置显示起始行 (0)                        */
    OLED_WriteCmd(0xA1);  /* 段重映射: column 127 = SEG0               */
    OLED_WriteCmd(0xC8);  /* COM 扫描方向: COM[N-1] → COM0             */
    OLED_WriteCmd(0xA6);  /* 正常显示 (非反转)                         */
    OLED_WriteCmd(0xA8);  /* 设置多路复用比                            */
    OLED_WriteCmd(0x3F);  /* 多路复用比 = 64 (64 行)                   */
    OLED_WriteCmd(0xD3);  /* 设置显示偏移                              */
    OLED_WriteCmd(0x00);  /* 偏移 = 0                                  */
    OLED_WriteCmd(0xD5);  /* 设置显示时钟分频 / 振荡器频率              */
    OLED_WriteCmd(0x80);  /* 分频=1, 频率≈8                            */
    OLED_WriteCmd(0xD9);  /* 设置预充电周期                            */
    OLED_WriteCmd(0xF1);  /* 预充电 = 15 个时钟, 放电 = 1 个时钟       */
    OLED_WriteCmd(0xDA);  /* 设置 COM 引脚硬件配置                     */
    OLED_WriteCmd(0x12);  /* 交替 COM 引脚配置                         */
    OLED_WriteCmd(0xDB);  /* 设置 VCOMH 电压                           */
    OLED_WriteCmd(0x40);  /* 约 0.77 × VCC                            */
    OLED_WriteCmd(0x8D);  /* 电荷泵设置                                */
    OLED_WriteCmd(0x14);  /* 使能电荷泵                                */
    OLED_WriteCmd(0xA4);  /* 全局显示: 输出 RAM 内容                   */
    OLED_WriteCmd(0xA6);  /* 正常显示                                  */
    OLED_WriteCmd(0xAF);  /* 开启显示 (Display ON)                     */

    /* ---- 清除屏幕 ---- */
    OLED_Clear();
}

/* ============================ 基础显示函数 ============================ */

/**
 * @brief  清除整个屏幕
 * @note   将所有 8 页 × 128 列的 GDDRAM 填充 0x00
 */
void OLED_Clear(void)
{
    uint8_t i, j;

    for (i = 0; i < OLED_PAGES; i++)
    {
        OLED_WriteCmd((uint8_t)(0xB0 + i));  /* 设置页地址 (0~7)       */
        OLED_WriteCmd(0x00);                  /* 设置列地址低4位        */
        OLED_WriteCmd(0x10);                  /* 设置列地址高4位        */

        /* 填充一行 128 个 0x00 */
        for (j = 0; j < OLED_WIDTH; j++)
        {
            OLED_Buffer[j] = 0x00;
        }
        OLED_WriteMultiData(OLED_Buffer, OLED_WIDTH);
    }

    /* 光标复位到 (0, 0) */
    OLED_Set_Pos(0, 0);
}

/**
 * @brief  设置光标位置
 * @param  x 列地址 (0~127)
 * @param  y 页地址 (0~7)
 */
void OLED_Set_Pos(uint8_t x, uint8_t y)
{
    OLED_WriteCmd((uint8_t)(0xB0 + y));      /* 页地址                 */
    OLED_WriteCmd((uint8_t)(x & 0x0F));      /* 列地址低4位            */
    OLED_WriteCmd((uint8_t)(0x10 | (x >> 4))); /* 列地址高4位          */
}

/* ============================ 字符/字符串显示 ============================ */

/**
 * @brief  在指定位置显示 6×8 字符
 * @param  x  列坐标
 * @param  y  页坐标
 * @param  ch ASCII 字符
 * @note   6x8 字体适合小字信息显示, 每行最多 21 个字符
 */
void OLED_ShowChar_6x8(uint8_t x, uint8_t y, uint8_t ch)
{
    const uint8_t *font_data = Font_Get_6x8(ch);
    uint8_t i;

    OLED_Set_Pos(x, y);

    for (i = 0; i < FONT6X8_BYTES; i++)
    {
        OLED_WriteData(font_data[i]);
    }
}

/**
 * @brief  在指定位置显示 6×8 字符串
 * @param  x   起始列坐标
 * @param  y   页坐标
 * @param  str 字符串
 */
void OLED_ShowString_6x8(uint8_t x, uint8_t y, const char *str)
{
    uint8_t pos_x = x;

    while (*str != '\0')
    {
        /* 越界换行处理 */
        if (pos_x + FONT_WIDTH_6X8 > OLED_WIDTH)
        {
            pos_x = 0;
            y += 2;  /* 每行占用 2 页 (16 像素高) */
        }

        /* 页面越界停止 */
        if (y > (OLED_PAGES - 1))
        {
            break;
        }

        OLED_ShowChar_6x8(pos_x, y, (uint8_t)*str);
        pos_x = (uint8_t)(pos_x + FONT_WIDTH_6X8);
        str++;
    }
}

/**
 * @brief  在指定位置显示 8×16 字符
 * @param  x  列坐标
 * @param  y  页坐标 (使用 y 和 y+1 两页)
 * @param  ch ASCII 字符
 * @note   8x16 字体适合标题显示, 每行最多 16 个字符
 */
void OLED_ShowChar_8x16(uint8_t x, uint8_t y, uint8_t ch)
{
    const uint8_t *font_data = Font_Get_8x16(ch);
    uint8_t i;

    /* 写入上半部分 (8 字节) */
    OLED_Set_Pos(x, y);
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(font_data[i]);
    }

    /* 写入下半部分 (8 字节) */
    OLED_Set_Pos(x, (uint8_t)(y + 1));
    for (i = 0; i < 8; i++)
    {
        OLED_WriteData(font_data[i + 8]);
    }
}

/**
 * @brief  在指定位置显示 8×16 字符串
 * @param  x   起始列坐标
 * @param  y   页坐标
 * @param  str 字符串
 */
void OLED_ShowString_8x16(uint8_t x, uint8_t y, const char *str)
{
    uint8_t pos_x = x;

    while (*str != '\0')
    {
        if (pos_x + FONT_WIDTH_8X16 > OLED_WIDTH)
        {
            pos_x = 0;
            y += 2;
        }

        if (y > (OLED_PAGES - 2))
        {
            break;
        }

        OLED_ShowChar_8x16(pos_x, y, (uint8_t)*str);
        pos_x = (uint8_t)(pos_x + FONT_WIDTH_8X16);
        str++;
    }
}

/* ============================ 屏幕控制 ============================ */

/**
 * @brief  显示/隐藏 OLED
 * @param  on 1=显示, 0=关闭
 */
void OLED_Display_On(uint8_t on)
{
    OLED_WriteCmd(on ? 0xAF : 0xAE);
}

/* ============================ 应用层显示函数 ============================ */

/**
 * @brief  显示系统启动画面
 * @note   显示项目名称、芯片型号和版本信息
 *         Line 0: " Smart Home Ctrl " (标题)
 *         Line 2: "STM32F103C8T6"   (芯片)
 *         Line 4: "Wokwi Simulation" (平台)
 *         Line 6: "  Version 1.0   "  (版本)
 */
void OLED_Show_BootScreen(void)
{
    OLED_Clear();

    /* Line 0: 标题 */
    OLED_ShowString_8x16(16, OLED_LINE_0, "Smart Home");
    OLED_ShowString_8x16(16, OLED_LINE_2, "  Control");
    OLED_ShowString_6x8( 24, OLED_LINE_4, "STM32F103C8T6");
    OLED_ShowString_6x8( 18, OLED_LINE_5, "Wokwi Simulation");
    OLED_ShowString_6x8( 30, OLED_LINE_7, "V1.0");
}

/**
 * @brief  更新主界面显示
 * @param  asr_result   语音识别结果字符串
 * @param  relay_status 4路继电器状态 (0=OFF, 1=ON)
 *
 * @note   主界面布局:
 *   Line 0 (Page 0): "==================================" (分隔线, 8x16字体)
 *     实际上使用 6x8 字体显示:
 *   Page 0: "=== Smart Home Ctrl ==="    (标题栏)
 *   Page 1: "Voice: xxxxxxxxxxxxxxxx"    (语音识别结果)
 *   Page 2: "1:O 2:X 3:O 4:X"           (设备状态, O=开 X=关)
 *   Page 3: (保留或显示提示)
 *   Page 4: "LGT FAN SKT SPR"           (设备标签)
 *   Page 6: "KEY5:AllON KEY6:AllOFF"    (按键提示)
 */
void OLED_Update_Main(const char *asr_result, const uint8_t *relay_status)
{
    char line_buf[OLED_STR_MAX_LEN + 1];
    const char *device_names[] = {"LIGHT", " FAN ", "SOCKT", "SPARE"};
    uint8_t i;

    /* ---- 清屏 ---- */
    OLED_Clear();

    /* ---- Page 0: 系统标题栏 ---- */
    OLED_ShowString_6x8(0, 0, "=== Smart Home Control ===");

    /* ---- Page 1: 语音识别结果显示 ---- */
    snprintf(line_buf, OLED_STR_MAX_LEN, "CMD: %s", asr_result);
    OLED_ShowString_6x8(0, 1, line_buf);

    /* ---- Page 2-3: 4 路设备状态 (每路 2 字符标签 + 状态) ---- */
    for (i = 0; i < 4; i++)
    {
        /* 每路设备占 30 像素宽 */
        snprintf(line_buf, OLED_STR_MAX_LEN, "%d.%s:%s",
                 i + 1,
                 device_names[i],
                 relay_status[i] ? "ON " : "OFF");

        if (i < 2)
        {
            OLED_ShowString_6x8((uint8_t)(i * 64), 2, line_buf);
        }
        else
        {
            OLED_ShowString_6x8((uint8_t)((i - 2) * 64), 3, line_buf);
        }
    }

    /* ---- Page 4-5: 操作提示 ---- */
    OLED_ShowString_6x8(0, 4, "Key:1-4=Toggle 5=AllON");
    OLED_ShowString_6x8(0, 5, "Key:6=AllOFF  UART=Voice");

    /* ---- Page 6-7: 系统状态信息 ---- */
    /* 计算系统运行秒数 */
    uint32_t uptime_sec = g_ms_counter / 1000;
    snprintf(line_buf, OLED_STR_MAX_LEN, "Run:%lus 9600-8N1",
             (unsigned long)uptime_sec);
    OLED_ShowString_6x8(0, 6, line_buf);

    snprintf(line_buf, OLED_STR_MAX_LEN, "STM32F103 72MHz OK");
    OLED_ShowString_6x8(0, 7, line_buf);
}
