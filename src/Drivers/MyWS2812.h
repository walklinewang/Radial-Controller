/*
 WS2812 LED 驱动封装头文件

 Copyright © 2026 Walkline Wang (walkline@gmail.com)
 Github: https://github.com/walklinewang/Radial-Controller
*/
#ifndef __MYWS2812_H__
#define __MYWS2812_H__

#include "../Common.h"
#include "EC11.h"
#include <Arduino.h>
#include <WS2812.h>

/**
 * @brief WS2812 LED 颜色顺序枚举
 */
typedef enum {
    WS2812_COLOR_ORDER_GRB, // 绿 > 红 > 蓝
    WS2812_COLOR_ORDER_RGB  // 红 > 绿 > 蓝
} ws2812_color_order_t;

/**
 * @brief WS2812 LED 状态枚举
 */
typedef enum {
    WS2812_EFFECT_STATE_ROTATION, // 流动灯效状态
    WS2812_EFFECT_STATE_FADE_IN,  // 渐亮状态
    WS2812_EFFECT_STATE_FADE_OUT  // 渐暗状态
} ws2812_effect_state_t;

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
    uint8_t pin;                         // LED 控制引脚
    uint8_t led_count;                   // 灯珠数量
    uint8_t led_data_size;               // LED 数据缓冲区实际大小
    uint8_t led_data[LED_COUNT_MAX * 3]; // LED 数据缓冲区
    uint8_t last_led_data[LED_COUNT_MAX * 3]; // 最后保存的 LED 颜色数据缓冲区
    ws2812_color_order_t color_order;   // 颜色顺序
    uint8_t brightness;                 // 亮度等级
    ws2812_effect_state_t effect_state; // 特效状态
    uint16_t rotate_interval;           // 流动灯效间隔时间
    uint16_t fade_duration;             // 渐变灯效持续时长
    uint32_t fade_start_time;           // 渐变灯效开始时间
} ws2812_t;

/**
 * @brief 初始化 WS2812 LED 驱动
 * @param pin LED 控制引脚
 * @param led_count 灯珠数量
 * @param color_order 颜色顺序
 * @return 初始化是否成功
 */
bool WS2812_Init(uint8_t pin, uint8_t led_count,
                 ws2812_color_order_t color_order);

/**
 * @brief 设置单个 LED 的颜色
 * @param index LED 索引
 * @param r 红色分量（0-255）
 * @param g 绿色分量（0-255）
 * @param b 蓝色分量（0-255）
 */
void WS2812_SetPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b);

/**
 * @brief 获取单个 LED 的颜色
 * @param index LED 索引
 * @param led_data LED 数据指针
 * @param r 指向红色分量存储的指针
 * @param g 指向绿色分量存储的指针
 * @param b 指向蓝色分量存储的指针
 */
void WS2812_GetPixel(uint8_t index, const uint8_t *led_data, uint8_t *r,
                     uint8_t *g, uint8_t *b);

/**
 * @brief 设置单个 LED 的颜色（使用颜色结构体）
 * @param index LED 索引
 * @param color 颜色结构体指针
 */
extern inline void WS2812_SetPixelColor(uint8_t index,
                                        const ws2812_color_t *color);

/**
 * @brief 设置所有 LED 为同一颜色
 * @param r 红色分量（0-255）
 * @param g 绿色分量（0-255）
 * @param b 蓝色分量（0-255）
 */
void WS2812_SetAllPixels(__data uint8_t r, __data uint8_t g, __data uint8_t b);

/**
 * @brief 清空所有 LED（设置为熄灭状态）
 */
void WS2812_Clear();

/**
 * @brief 将 LED 数据显示到灯珠上
 */
void WS2812_Show();

/**
 * @brief 设置 LED 流动灯效触发间隔
 * @param interval 触发间隔（毫秒）
 */
void WS2812_SetRotateEffectInterval(uint16_t interval);

/**
 * @brief 执行 LED 流动灯效
 * @param direction 旋转方向（EC11_DIR_CW 顺时针, EC11_DIR_CCW 逆时针）
 */
void WS2812_ShowRotationEffect(__data ec11_direction_t direction);

/**
 * @brief 设置 LED 渐亮效果
 * @param duration 持续时长（毫秒）
 */
void WS2812_SetFadeInEffect();

/**
 * @brief 设置 LED 渐暗效果
 * @param duration 持续时长（毫秒）
 */
void WS2812_SetFadeOutEffect();

/**
 * @brief 设置 LED 渐变灯效持续时长
 * @param duration 持续时长（毫秒）
 */
void WS2812_SetFadeEffectDuration(uint16_t duration);

/**
 * @brief 执行 LED 渐变灯效
 */
void WS2812_ShowFadeEffect();

/**
 * @brief 获取当前 LED 特效状态
 * @return 当前状态
 */
ws2812_effect_state_t WS2812_GetEffectState();

/**
 * @brief 设置当前亮度等级
 * @param level 亮度等级（0-4），0 为最暗，4 为最亮
 */
void WS2812_SetBrightness(uint8_t level);

/**
 * @brief 获取当前亮度等级
 * @return 当前亮度等级（0-4），0 为最暗，4 为最亮
 */
uint8_t WS2812_GetBrightness();

#endif /* __MYWS2812_H__ */
