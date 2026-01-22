/*
 WS2812 LED 驱动封装实现文件

 Copyright © 2026 Walkline Wang (walkline@gmail.com)
 Github: https://github.com/walklinewang/Radial-Controller
*/
#include "MyWS2812.h"

#define GRADIENT_STEPS 30 // 颜色渐变总步数（可选30、60、90等）
#define GRADIENT_SEGMENT_RG (GRADIENT_STEPS / 3) // 红→绿渐变分段点
#define GRADIENT_SEGMENT_GB (GRADIENT_SEGMENT_RG * 2) // 绿→蓝渐变分段点
#define GRADIENT_STEP_SIZE (255 / GRADIENT_SEGMENT_RG) // 颜色渐变步长

static const __code uint8_t BRIGHT_LEVELS[BRIGHTNESS_MAX + 1] = {0, 80, 120,
                                                                 160, 200};
static __xdata ws2812_t ws2812;

/**
 * @brief 初始化 WS2812 LED 驱动
 * @param pin LED 控制引脚
 * @param led_count 灯珠数量
 * @param color_order 颜色顺序
 * @return 初始化是否成功
 */
bool WS2812_Init(uint8_t pin, uint8_t led_count,
                 ws2812_color_order_t color_order) {
    if (led_count > LED_COUNT_MAX) {
        return false;
    }

    // 设置基本参数
    ws2812.pin = pin;
    ws2812.led_count = led_count;
    ws2812.led_data_size = led_count * 3; // 每个 LED 需要 3 个字节
    ws2812.color_order = color_order;
    ws2812.brightness = BRIGHTNESS_DEFAULT;
    ws2812.effect_state = WS2812_EFFECT_STATE_ROTATION;
    ws2812.rotate_interval = ROTATE_INTERVAL_DEFAULT;
    ws2812.fade_duration = FADE_DURATION_DEFAULT;
    ws2812.fade_start_time = 0;

    // 设置引脚为输出模式
    pinMode(ws2812.pin, OUTPUT);

    return true;
}

/**
 * @brief 设置单个 LED 的颜色
 * @param index LED 索引
 * @param r 红色分量（0-255）
 * @param g 绿色分量（0-255）
 * @param b 蓝色分量（0-255）
 */
void WS2812_SetPixel(uint8_t index, uint8_t r, uint8_t g, uint8_t b) {
    if (index >= ws2812.led_count) {
        return;
    }

    switch (ws2812.color_order) {
    case WS2812_COLOR_ORDER_GRB:
        set_pixel_for_GRB_LED(ws2812.led_data, index, r, g, b);
        break;
    case WS2812_COLOR_ORDER_RGB:
        set_pixel_for_RGB_LED(ws2812.led_data, index, r, g, b);
        break;
    default:
        break;
    }
}

/**
 * @brief 获取单个 LED 的颜色
 * @param index LED 索引
 * @param led_data LED 数据指针
 * @param r 指向红色分量存储的指针
 * @param g 指向绿色分量存储的指针
 * @param b 指向蓝色分量存储的指针
 */
void WS2812_GetPixel(uint8_t index, const uint8_t *led_data, uint8_t *r,
                     uint8_t *g, uint8_t *b) {
    if (index >= ws2812.led_count) {
        return;
    }

    const uint8_t *ptr = led_data + (index * 3);

    switch (ws2812.color_order) {
    case WS2812_COLOR_ORDER_GRB:
        *r = ptr[1];
        *g = ptr[0];
        *b = ptr[2];
        break;
    case WS2812_COLOR_ORDER_RGB:
        *r = ptr[0];
        *g = ptr[1];
        *b = ptr[2];
        break;
    default:
        break;
    }
}

/**
 * @brief 设置单个 LED 的颜色
 * @param index LED 索引
 * @param color 颜色结构体指针
 */
inline void WS2812_SetPixelColor(uint8_t index, const ws2812_color_t *color) {
    WS2812_SetPixel(index, color->r, color->g, color->b);
}

/**
 * @brief 设置所有 LED 为同一颜色，并立即显示
 * @param r 红色分量（0-255）
 * @param g 绿色分量（0-255）
 * @param b 蓝色分量（0-255）
 */
void WS2812_SetAllPixels(__data uint8_t r, __data uint8_t g, __data uint8_t b) {
    for (uint8_t i = 0; i < ws2812.led_count; i++) {
        WS2812_SetPixel(i, r, g, b);
    }

    WS2812_Show();
}

/**
 * @brief 清空所有 LED 数据，并立即灭灯
 */
void WS2812_Clear() {
    // 添加短暂延时确保硬件稳定，避免时序冲突
    delayMicroseconds(10);

    memset(ws2812.led_data, 0, ws2812.led_data_size);
    WS2812_Show();
}

/**
 * @brief 将 LED 数据显示到灯珠上
 */
void WS2812_Show() {
// 根据引脚号选择对应的显示函数
#if WS2812_PIN == 10 // P1_0
    neopixel_show_P1_0(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 11 // P1_1
    neopixel_show_P1_1(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 12 // P1_2
    neopixel_show_P1_2(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 13 // P1_3
    neopixel_show_P1_3(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 14 // P1_4
    neopixel_show_P1_4(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 15 // P1_5
    neopixel_show_P1_5(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 16 // P1_6
    neopixel_show_P1_6(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 17 // P1_7
    neopixel_show_P1_7(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 30 // P3_0
    neopixel_show_P3_0(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 31 // P3_1
    neopixel_show_P3_1(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 32 // P3_2
    neopixel_show_P3_2(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 33 // P3_3
    neopixel_show_P3_3(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 34 // P3_4
    neopixel_show_P3_4(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 35 // P3_5
    neopixel_show_P3_5(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 36 // P3_6
    neopixel_show_P3_6(ws2812.led_data, ws2812.led_data_size);
#elif WS2812_PIN == 37 // P3_7
    neopixel_show_P3_7(ws2812.led_data, ws2812.led_data_size);
#else
#error "WS2812_PIN 未定义或不支持"
#endif
}

/**
 * @brief 填充 LED 流动灯效颜色数据
 * @param index LED 索引
 * @param offset 颜色偏移量
 */
static void WS2812_FillRotationEffectColor(__data uint8_t index,
                                           __data uint8_t offset) {
    __data uint8_t r, g, b;

    if (offset < GRADIENT_SEGMENT_RG) {
        // 红 → 绿渐变
        r = 255 - offset * GRADIENT_STEP_SIZE;
        g = offset * GRADIENT_STEP_SIZE;
        b = 0;
    } else if (offset < GRADIENT_SEGMENT_GB) {
        // 绿 → 蓝渐变
        g = 255 - (offset - GRADIENT_SEGMENT_RG) * GRADIENT_STEP_SIZE;
        b = (offset - GRADIENT_SEGMENT_RG) * GRADIENT_STEP_SIZE;
        r = 0;
    } else {
        // 蓝 → 红渐变
        b = 255 - (offset - GRADIENT_SEGMENT_GB) * GRADIENT_STEP_SIZE;
        r = (offset - GRADIENT_SEGMENT_GB) * GRADIENT_STEP_SIZE;
        g = 0;
    }

    // 应用亮度调整
    __data uint8_t brightness = BRIGHT_LEVELS[ws2812.brightness];
    r = (r * brightness) / 255;
    g = (g * brightness) / 255;
    b = (b * brightness) / 255;

    WS2812_SetPixel(index, r, g, b);
}

/**
 * @brief 设置 LED 流动灯效触发间隔
 * @param interval 触发间隔（毫秒）
 */
void WS2812_SetRotateEffectInterval(uint16_t interval) {
    ws2812.rotate_interval = interval;
}

/**
 * @brief 执行 LED 流动灯效
 * @param direction 旋转方向（EC11_DIR_CW 顺时针, EC11_DIR_CCW 逆时针）
 */
void WS2812_ShowRotationEffect(__data ec11_direction_t direction) {
    static __data uint8_t count = 0;
    static __data uint32_t last_effect_time = 0;

    if (millis() - last_effect_time < ws2812.rotate_interval) {
        return; // 时间间隔未到，不执行特效
    }

    last_effect_time = millis();

    // 如果 LED 与 EC11 编码器在电路板同侧，则需要调整计数方向为 EC11_DIR_CW
    if (direction == EC11_DIR_CCW) {
        count = (count < GRADIENT_STEPS - 1) ? count + 1 : 0;
    } else {
        count = (count > 0) ? count - 1 : GRADIENT_STEPS - 1;
    }

    // 根据 LED 数量动态计算色相间隔，确保颜色均匀分布
    for (uint8_t index = 0; index < ws2812.led_count; index++) {
        WS2812_FillRotationEffectColor(
            index, (count + (index * GRADIENT_STEPS) / ws2812.led_count) %
                       GRADIENT_STEPS);
    }

    WS2812_Show();
}

/**
 * @brief 设置 LED 渐亮效果
 */
void WS2812_SetFadeInEffect() {
    ws2812.effect_state = WS2812_EFFECT_STATE_FADE_IN;
    ws2812.fade_start_time = millis();
}

/**
 * @brief 设置 LED 渐暗效果
 */
void WS2812_SetFadeOutEffect() {
    // 保存当前 LED 颜色到原始数据缓冲区
    memcpy(ws2812.last_led_data, ws2812.led_data, ws2812.led_data_size);

    ws2812.effect_state = WS2812_EFFECT_STATE_FADE_OUT;
    ws2812.fade_start_time = millis();
}

/**
 * @brief 设置 LED 渐变灯效持续时长
 * @param duration 持续时长（毫秒）
 */
void WS2812_SetFadeEffectDuration(uint16_t duration) {
    ws2812.fade_duration = duration;
}

/**
 * @brief 执行 LED 渐变灯效
 */
void WS2812_ShowFadeEffect() {
    if (ws2812.effect_state == WS2812_EFFECT_STATE_ROTATION) {
        return;
    }

    __data uint32_t elapsed_time = millis() - ws2812.fade_start_time;

    // 计算渐变进度
    __data uint16_t progress = (elapsed_time * 255) / ws2812.fade_duration;

    // 确保进度不超过255
    if (progress > 255) {
        progress = 255;
    }

    __data uint8_t target_r, target_g, target_b;

    // 根据当前状态计算颜色
    if (ws2812.effect_state == WS2812_EFFECT_STATE_FADE_IN) {
        // 渐亮：从 0 到最后保存的颜色
        for (uint8_t i = 0; i < ws2812.led_count; i++) {
            WS2812_GetPixel(i, ws2812.last_led_data, &target_r, &target_g,
                            &target_b);

            // 计算渐变颜色
            target_r = (uint8_t)((target_r * progress) >> 8);
            target_g = (uint8_t)((target_g * progress) >> 8);
            target_b = (uint8_t)((target_b * progress) >> 8);

            WS2812_SetPixel(i, target_r, target_g, target_b);
        }
    } else if (ws2812.effect_state == WS2812_EFFECT_STATE_FADE_OUT) {
        // 渐暗：从最后保存的颜色到 0
        for (uint8_t i = 0; i < ws2812.led_count; i++) {
            WS2812_GetPixel(i, ws2812.last_led_data, &target_r, &target_g,
                            &target_b);

            // 计算渐变颜色
            target_r = (uint8_t)((target_r * (255 - progress)) >> 8);
            target_g = (uint8_t)((target_g * (255 - progress)) >> 8);
            target_b = (uint8_t)((target_b * (255 - progress)) >> 8);

            WS2812_SetPixel(i, target_r, target_g, target_b);
        }
    }

    WS2812_Show();

    // 检查是否完成渐变
    if (elapsed_time >= ws2812.fade_duration) {
        if (ws2812.effect_state == WS2812_EFFECT_STATE_FADE_IN) {
            ws2812.effect_state = WS2812_EFFECT_STATE_ROTATION;
        } else if (ws2812.effect_state == WS2812_EFFECT_STATE_FADE_OUT) {
            // WS2812_Clear();
        }
    }
}

/**
 * @brief 获取当前 LED 特效状态
 * @return 当前状态
 */
ws2812_effect_state_t WS2812_GetEffectState() { return ws2812.effect_state; }

/**
 * @brief 设置当前亮度等级
 * @param level 亮度等级
 */
void WS2812_SetBrightness(uint8_t level) {
    if (level <= BRIGHTNESS_MAX) {
        ws2812.brightness = level;
    }
}

/**
 * @brief 获取当前亮度等级
 * @return 当前亮度等级
 */
uint8_t WS2812_GetBrightness() { return ws2812.brightness; }
