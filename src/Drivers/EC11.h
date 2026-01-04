/*
  EC11 编码器驱动头文件

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#ifndef __EC11_H__
#define __EC11_H__

#include <Arduino.h>

/**
 * @brief EC11 编码器旋转方向枚举
 */
typedef enum {
    EC11_DIR_NONE, // 无旋转
    EC11_DIR_CW,   // 顺时针旋转
    EC11_DIR_CCW   // 逆时针旋转
} ec11_direction_t;

/**
 * @brief EC11 编码器按键状态枚举
 */
typedef enum {
    EC11_KEY_RELEASED, // 按键释放
    EC11_KEY_PRESSED   // 按键按下
} ec11_key_state_t;

/**
 * @brief EC11 编码器结构体
 */
typedef struct {
    uint8_t pin_a;              // A 相引脚
    uint8_t pin_b;              // B 相引脚
    uint8_t pin_key;            // 按键引脚
    uint8_t last_a_state;       // 上一次 A 相状态
    uint8_t last_b_state;       // 上一次 B 相状态
    uint8_t last_key_state;     // 上一次按键状态
    ec11_direction_t direction; // 当前旋转方向
    ec11_key_state_t key_state; // 当前按键状态
    bool key_changed;           // 按键状态是否变化
} ec11_t;

/**
 * @brief 初始化 EC11 编码器
 * @param pin_a A 相引脚
 * @param pin_b B 相引脚
 * @param pin_key 按键引脚
 */
void EC11_Init(uint8_t pin_a, uint8_t pin_b, uint8_t pin_key);

/**
 * @brief 更新 EC11 编码器状态
 */
void EC11_UpdateStatus(void);

/**
 * @brief 获取 EC11 编码器旋转方向
 * @return 旋转方向
 */
ec11_direction_t EC11_GetDirection(void);

/**
 * @brief 获取 EC11 编码器按键状态
 * @return 按键状态
 */
ec11_key_state_t EC11_GetKeyState(void);

/**
 * @brief 检查 EC11 编码器按键状态是否变化
 * @return 是否变化
 */
bool EC11_IsKeyChanged(void);

#endif /* __EC11_H__ */