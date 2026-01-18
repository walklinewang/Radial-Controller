/*
  EC11 编码器驱动头文件

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#ifndef __EC11_H__
#define __EC11_H__

#include "../Common.h"
#include <Arduino.h>

/**
 * @brief EC11 编码器相位配置枚举
 * @details 用于指定 EC11 编码器的硬件相位配置
 *          - EC11_PHASE_A_LEADS: A 相超前（默认配置）
 *          - EC11_PHASE_B_LEADS: B 相超前
 * @note 无论硬件实际配置如何，软件内部都会统一转换为 A 相超前的逻辑处理
 */
typedef enum {
    EC11_PHASE_A_LEADS, // A 相超前
    EC11_PHASE_B_LEADS  // B 相超前
} ec11_phase_t;

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
    uint8_t step_per_teeth;     // 转动一齿触发次数
    ec11_phase_t phase;         // 相位配置
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
void EC11_UpdateStatus();

/**
 * @brief 获取 EC11 编码器旋转方向
 * @return 旋转方向
 */
extern inline ec11_direction_t EC11_GetDirection();

/**
 * @brief 获取 EC11 编码器按键状态
 * @return 按键状态
 */
ec11_key_state_t EC11_GetKeyState();

/**
 * @brief 检查 EC11 编码器按键状态是否变化
 * @return 是否变化
 */
bool EC11_IsKeyChanged();

/**
 * @brief 设置 EC11 编码器触发动作的次数
 * @param step 每转动一齿触发动作次数
 */
void EC11_SetStepPerTeeth(uint8_t step);

/**
 * @brief 设置 EC11 编码器相位配置
 * @param phase 相位配置
 */
void EC11_SetPhase(ec11_phase_t phase);

#endif /* __EC11_H__ */
