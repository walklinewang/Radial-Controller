/*
  EC11 编码器驱动实现文件

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#include "EC11.h"

static ec11_t encoder;

/**
 * @brief 初始化 EC11 编码器
 * @param pin_a A 相引脚
 * @param pin_b B 相引脚
 * @param pin_key 按键引脚
 */
void EC11_Init(uint8_t pin_a, uint8_t pin_b, uint8_t pin_key) {
    // 设置引脚参数
    encoder.pin_a = pin_a;
    encoder.pin_b = pin_b;
    encoder.pin_key = pin_key;

    // 初始化状态
    encoder.direction = EC11_DIR_NONE;
    encoder.key_state = EC11_KEY_RELEASED;
    encoder.key_changed = false;
    encoder.step_per_teeth = 2; // 默认转动一齿触发2次

    // 设置引脚模式
    pinMode(encoder.pin_a, INPUT_PULLUP);
    pinMode(encoder.pin_b, INPUT_PULLUP);
    pinMode(encoder.pin_key, INPUT_PULLUP);

    // 读取初始状态
    encoder.last_a_state = digitalRead(encoder.pin_a);
    encoder.last_b_state = digitalRead(encoder.pin_b);
    encoder.last_key_state = digitalRead(encoder.pin_key);
}

/**
 * @brief 更新 EC11 编码器状态
 */
void EC11_UpdateStatus() {
    static uint8_t count = 0;

    // 初始化方向为无旋转
    encoder.direction = EC11_DIR_NONE;

    // 读取当前状态
    uint8_t current_a_state = digitalRead(encoder.pin_a);
    uint8_t current_b_state = digitalRead(encoder.pin_b);
    uint8_t current_key_state = digitalRead(encoder.pin_key);

    // 检测旋转方向（基于 A 相的变化）
    if (encoder.last_a_state != current_a_state) {
        count += (current_b_state != current_a_state) ? 1 : -1;
    }

    // 根据 step_per_teeth 配置判断是否触发旋转事件
    uint8_t threshold = (encoder.step_per_teeth == 1) ? 2 : 1;

    if (count >= threshold) {
        encoder.direction = EC11_DIR_CW; // 顺时针旋转
        count = 0;
    } else if (count <= -threshold) {
        encoder.direction = EC11_DIR_CCW; // 逆时针旋转
        count = 0;
    }

    // 更新 A 相和 B 相的上一次状态
    encoder.last_a_state = current_a_state;
    encoder.last_b_state = current_b_state;

    // 检测按键状态变化
    encoder.key_changed = (encoder.last_key_state != current_key_state);

    // 更新按键状态
    if (encoder.key_changed) {
        encoder.key_state =
            (current_key_state == LOW) ? EC11_KEY_PRESSED : EC11_KEY_RELEASED;
        encoder.last_key_state = current_key_state;
    }
}

/**
 * @brief 获取 EC11 编码器旋转方向
 * @return 旋转方向
 */
ec11_direction_t EC11_GetDirection() { return encoder.direction; }

/**
 * @brief 获取 EC11 编码器按键状态
 * @return 按键状态
 */
ec11_key_state_t EC11_GetKeyState() { return encoder.key_state; }

/**
 * @brief 检查 EC11 编码器按键状态是否变化
 * @return 是否变化
 */
bool EC11_IsKeyChanged() { return encoder.key_changed; }

/**
 * @brief 设置 EC11 编码器触发动作的次数
 * @param step 每转动一齿触发动作次数
 */
void EC11_SetStepPerTeeth(uint8_t step) {
    if (step == 1 || step == 2) {
        encoder.step_per_teeth = step;
    }
}
