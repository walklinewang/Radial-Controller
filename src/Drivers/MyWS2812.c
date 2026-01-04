/*
 WS2812 LED驱动封装实现文件

 Copyright © 2026 Walkline Wang (walkline@gmail.com)
 Github: https://github.com/walklinewang/Radial-Controller
*/
#include "MyWS2812.h"

static const uint8_t BRIGHT_LEVELS[5] = {15, 9, 5, 3, 0};
static ws2812_t ws2812;
static uint8_t brightness = 1;

/**
 * @brief 初始化 WS2812 LED 驱动
 * @param pin LED 控制引脚
 * @param led_count 灯珠数量
 * @param color_order 颜色顺序 (0: GRB, 1: RGB)
 * @return 初始化是否成功
 */
bool WS2812_Init(uint8_t pin, uint8_t led_count, uint8_t color_order) {
    // 检查灯珠数量是否超过最大支持数量
    if (led_count > 10) {
        return false;
    }

    // 设置基本参数
    ws2812.pin = pin;
    ws2812.led_count = led_count;
    ws2812.color_order = color_order;
    ws2812.data_size = led_count * 3; // 每个 LED 需要 3 个字节

    // 设置引脚为输出模式
    pinMode(ws2812.pin, OUTPUT);

    // 清空LED数据
    WS2812_Clear();

    return true;
}

/**
 * @brief 释放 WS2812 资源
 */
void WS2812_Deinit(void) {
    // 清空 LED 数据
    WS2812_Clear();

    // 重置参数
    ws2812.pin = 0;
    ws2812.led_count = 0;
    ws2812.color_order = 0;
    ws2812.data_size = 0;
}

/**
 * @brief 设置单个 LED 的颜色
 * @param index LED 索引
 * @param r 红色分量 (0-255)
 * @param g 绿色分量 (0-255)
 * @param b 蓝色分量 (0-255)
 */
void WS2812_SetPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= ws2812.led_count) {
        return;
    }

    uint8_t offset = index * 3;

    if (ws2812.color_order == 0) { // GRB 格式
        ws2812.led_data[offset] = g;
        ws2812.led_data[offset + 1] = r;
        ws2812.led_data[offset + 2] = b;
    } else { // RGB 格式
        ws2812.led_data[offset] = r;
        ws2812.led_data[offset + 1] = g;
        ws2812.led_data[offset + 2] = b;
    }
}

/**
 * @brief 设置单个 LED 的颜色（使用颜色结构体）
 * @param index LED 索引
 * @param color 颜色结构体
 */
void WS2812_SetPixelColor(uint8_t index, ws2812_color_t color) {
    WS2812_SetPixel(index, color.r, color.g, color.b);
}

/**
 * @brief 填充 LED 流动灯效颜色
 * @param index LED 索引
 * @param offset 颜色偏移量
 */
static void WS2812_FillEffectColor(uint8_t index, uint8_t offset) {
    uint8_t temp;
    ws2812_color_t color;

    temp = offset / 10;
    offset %= 10;
    offset *= BRIGHT_LEVELS[brightness];

    switch (temp) {
    case 0:
        color.r = BRIGHT_LEVELS[brightness] * 10 - offset;
        color.g = offset;
        color.b = 0;
        WS2812_SetPixelColor(index, color);
        break;
    case 1:
        color.g = BRIGHT_LEVELS[brightness] * 10 - offset;
        color.b = offset;
        color.r = 0;
        WS2812_SetPixelColor(index, color);
        break;
    default:
        color.b = BRIGHT_LEVELS[brightness] * 10 - offset;
        color.r = offset;
        color.g = 0;
        WS2812_SetPixelColor(index, color);
        break;
    }
}

/**
 * @brief 设置 LED 流动灯效
 * @param direction 旋转方向 (EC11_DIR_CW 顺时针, EC11_DIR_CCW 逆时针)
 */
void WS2812_SetEffectColor(ec11_direction_t direction) {
    static uint8_t count = 0;

    if (direction == EC11_DIR_CW) {
        count = (count < 29) ? count + 1 : 0;
    } else {
        count = (count > 0) ? count - 1 : 29;
    }

    for (uint8_t index = 0; index < ws2812.led_count; index++) {
        WS2812_FillEffectColor(index,
                               (count + (index * 30) / ws2812.led_count) % 30);
    }
}

/**
 * @brief 将 LED 数据显示到灯珠上
 */
void WS2812_Show(void) {
    // 根据引脚号调用对应的显示函数
    switch (ws2812.pin) {
    case 10: // P1_0
        neopixel_show_P1_0(ws2812.led_data, ws2812.data_size);
        break;
    case 11: // P1_1
        neopixel_show_P1_1(ws2812.led_data, ws2812.data_size);
        break;
    case 12: // P1_2
        neopixel_show_P1_2(ws2812.led_data, ws2812.data_size);
        break;
    case 13: // P1_3
        neopixel_show_P1_3(ws2812.led_data, ws2812.data_size);
        break;
    case 14: // P1_4
        neopixel_show_P1_4(ws2812.led_data, ws2812.data_size);
        break;
    case 15: // P1_5
        neopixel_show_P1_5(ws2812.led_data, ws2812.data_size);
        break;
    case 16: // P1_6
        neopixel_show_P1_6(ws2812.led_data, ws2812.data_size);
        break;
    case 17: // P1_7
        neopixel_show_P1_7(ws2812.led_data, ws2812.data_size);
        break;
    case 30: // P3_0
        neopixel_show_P3_0(ws2812.led_data, ws2812.data_size);
        break;
    case 31: // P3_1
        neopixel_show_P3_1(ws2812.led_data, ws2812.data_size);
        break;
    case 32: // P3_2
        neopixel_show_P3_2(ws2812.led_data, ws2812.data_size);
        break;
    case 33: // P3_3
        neopixel_show_P3_3(ws2812.led_data, ws2812.data_size);
        break;
    case 34: // P3_4
        neopixel_show_P3_4(ws2812.led_data, ws2812.data_size);
        break;
    case 35: // P3_5
        neopixel_show_P3_5(ws2812.led_data, ws2812.data_size);
        break;
    case 36: // P3_6
        neopixel_show_P3_6(ws2812.led_data, ws2812.data_size);
        break;
    case 37: // P3_7
        neopixel_show_P3_7(ws2812.led_data, ws2812.data_size);
        break;
    }
}

/**
 * @brief 清空所有 LED 数据
 */
void WS2812_Clear(void) {
    for (uint8_t i = 0; i < ws2812.data_size; i++) {
        ws2812.led_data[i] = 0;
    }

    WS2812_Show();
}

/**
 * @brief 设置所有 LED 为同一颜色
 * @param r 红色分量 (0-255)
 * @param g 绿色分量 (0-255)
 * @param b 蓝色分量 (0-255)
 */
void WS2812_SetAllPixels(uint8_t r, uint8_t g, uint8_t b) {
    for (uint8_t i = 0; i < ws2812.led_count; i++) {
        WS2812_SetPixel(i, r, g, b);
    }

    WS2812_Show();
}

/**
 * @brief 设置当前亮度等级
 * @param level 亮度等级 (0-4)，0 为最亮，4 为最暗
 */
void WS2812_SetBrightness(uint8_t level) {
    if (level < 5) { // 限制亮度等级在 0~4 范围内
        brightness = level;
    }
}

/**
 * @brief 获取当前亮度等级
 * @return 当前亮度等级 (0-4)，0 为最亮，4 为最暗
 */
uint8_t WS2812_GetBrightness(void) { return brightness; }
