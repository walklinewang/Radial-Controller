/*
 WS2812 LED 驱动封装头文件

 Copyright © 2026 Walkline Wang (walkline@gmail.com)
 Github: https://github.com/walklinewang/Radial-Controller
*/
#ifndef __MYWS2812_H__
#define __MYWS2812_H__

#include "EC11.h"
#include <Arduino.h>
#include <WS2812.h>

/**
 * @brief WS2812 LED 颜色结构体
 */
typedef struct {
    uint8_t r;
    uint8_t g;
    uint8_t b;
} ws2812_color_t;

/**
 * @brief WS2812 LED 结构体
 */
typedef struct {
    uint8_t pin;       // LED 控制引脚
    uint8_t led_count; // 灯珠数量
    uint8_t led_data[30]; // LED 数据缓冲区 (最大支持 10 个 LED，10×3=30 字节)
    uint8_t color_order; // 颜色顺序 (0: GRB, 1: RGB)
    uint16_t data_size;  // 数据缓冲区大小
} ws2812_t;

/**
 * @brief 初始化 WS2812 LED 驱动
 * @param pin LED 控制引脚
 * @param led_count 灯珠数量
 * @param color_order 颜色顺序 (0: GRB, 1: RGB)
 * @return 初始化是否成功
 */
bool WS2812_Init(uint8_t pin, uint8_t led_count, uint8_t color_order);

/**
 * @brief 释放 WS2812 资源
 */
void WS2812_Deinit(void);

/**
 * @brief 设置单个 LED 的颜色
 * @param index LED 索引
 * @param r 红色分量 (0-255)
 * @param g 绿色分量 (0-255)
 * @param b 蓝色分量 (0-255)
 */
void WS2812_SetPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 设置单个 LED 的颜色（使用颜色结构体）
 * @param index LED 索引
 * @param color 颜色结构体
 */
void WS2812_SetPixelColor(uint8_t index, ws2812_color_t color);

/**
 * @brief 设置 LED 流动灯效
 * @param direction 旋转方向 (EC11_DIR_CW 顺时针, EC11_DIR_CCW 逆时针)
 */
void WS2812_SetEffectColor(ec11_direction_t direction);

/**
 * @brief 将 LED 数据显示到灯珠上
 */
void WS2812_Show(void);

/**
 * @brief 清空所有 LED（设置为熄灭状态）
 */
void WS2812_Clear(void);

/**
 * @brief 设置所有 LED 为同一颜色
 * @param r 红色分量 (0-255)
 * @param g 绿色分量 (0-255)
 * @param b 蓝色分量 (0-255)
 */
void WS2812_SetAllPixels(uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 设置当前亮度等级
 * @param level 亮度等级 (0-4)，0 为最亮，4 为最暗
 */
void WS2812_SetBrightness(uint8_t level);

/**
 * @brief 获取当前亮度等级
 * @return 当前亮度等级 (0-4)，0 为最亮，4 为最暗
 */
uint8_t WS2812_GetBrightness(void);

#endif /* __MYWS2812_H__ */
