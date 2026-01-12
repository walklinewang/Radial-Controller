/*
  EEPROM 配置参数存储驱动头文件

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#ifndef __EEPROM_H__
#define __EEPROM_H__

#include <Arduino.h>

// 配置参数存储在EEPROM中的起始地址
#define EEPROM_CONFIG_START_ADDRESS 0

// 配置版本号和修订号
#define CURRENT_CONFIG_VERSION 0
#define CURRENT_CONFIG_REVISION 1

// 配置结构体大小（字节）
#define CONFIG_STRUCT_SIZE sizeof(config_t)

/**
 * @brief EEPROM 操作状态枚举
 */
typedef enum {
    EEPROM_STATUS_OK,           // 操作成功
    EEPROM_STATUS_ERROR,        // 操作错误
    EEPROM_STATUS_INVALID_PARAM // 无效参数
} eeprom_status_t;

/**
 * @brief 设备配置参数结构体
 */
typedef struct {
    uint8_t version;  // 版本号 (0)
    uint8_t revision; // 修订号 (1)

    uint8_t led_count;   // LED 灯珠数量 (2)
    uint8_t color_order; // LED 颜色顺序（0:GRB, 1:RGB） (3)
    uint8_t brightness;  // 亮度等级（0-4） (4)

    uint8_t effect_mode;  // LED 灯效模式 (5)
    uint16_t effect_tick; // LED 灯效循环周期（ms） (6-7)

    int16_t rotate_cw;      // 顺时针旋转角度 (8-9)
    int16_t rotate_ccw;     // 逆时针旋转角度 (10-11)
    uint8_t step_per_teeth; // 转动一齿触发次数 (12)

    uint8_t reserved[19]; // 预留空间，用于未来扩展 (13-31)
} config_t;               /* 共 32 字节*/

/**
 * @brief 获取完整的配置结构体数据
 * @return 配置结构体指针
 */
config_t *EEPROM_GetConfigData(void);

/**
 * @brief 从 EEPROM 读取配置参数
 * @return 操作状态
 */
eeprom_status_t EEPROM_LoadConfig();

/**
 * @brief 将配置参数写入 EEPROM
 * @return 操作状态
 */
eeprom_status_t EEPROM_SaveConfig();

/**
 * @brief 重置配置参数为默认值
 * @return 操作状态
 */
eeprom_status_t EEPROM_Reset();

/**
 * @brief 验证配置参数的有效性
 * @return 操作状态
 */
eeprom_status_t EEPROM_Validate();

/**
 * @brief 获取版本号
 * @return 版本号
 */
uint8_t EEPROM_GetVersion(void);

/**
 * @brief 获取修订号
 * @return 修订号
 */
uint8_t EEPROM_GetRevision(void);

/**
 * @brief 获取 LED 灯珠数量
 * @return LED 灯珠数量
 */
uint8_t EEPROM_GetLedCount(void);

/**
 * @brief 设置 LED 灯珠数量
 * @param count LED 灯珠数量
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetLedCount(uint8_t count);

/**
 * @brief 获取 LED 颜色顺序
 * @return LED 颜色顺序（0:GRB, 1:RGB）
 */
uint8_t EEPROM_GetColorOrder(void);

/**
 * @brief 设置 LED 颜色顺序
 * @param order LED 颜色顺序（0:GRB, 1:RGB）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetColorOrder(uint8_t order);

/**
 * @brief 获取亮度等级
 * @return 亮度等级（0-4）
 */
uint8_t EEPROM_GetBrightness(void);

/**
 * @brief 设置 LED 亮度等级
 * @param brightness 亮度等级（0-4）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetBrightness(uint8_t brightness);

/**
 * @brief 获取 LED 灯效模式
 * @return LED 灯效模式
 */
uint8_t EEPROM_GetEffectMode(void);

/**
 * @brief 设置 LED 灯效模式
 * @param mode LED 灯效模式
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetEffectMode(uint8_t mode);

/**
 * @brief 获取 LED 灯效循环周期
 * @return LED 灯效循环周期（ms）
 */
uint16_t EEPROM_GetEffectTick(void);

/**
 * @brief 设置 LED 灯效循环周期
 * @param tick LED 灯效循环周期（ms）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetEffectTick(uint16_t tick);

/**
 * @brief 获取顺时针旋转角度
 * @return 顺时针旋转角度
 */
int16_t EEPROM_GetRotateCW(void);

/**
 * @brief 设置 LED 顺时针旋转角度
 * @param degrees 顺时针旋转角度
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetRotateCW(int16_t degrees);

/**
 * @brief 获取 LED 逆时针旋转角度
 * @return 逆时针旋转角度
 */
int16_t EEPROM_GetRotateCCW(void);

/**
 * @brief 设置 LED 逆时针旋转角度
 * @param degrees 逆时针旋转角度
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetRotateCCW(int16_t degrees);

/**
 * @brief 获取触发动作的次数
 * @return EC11 编码器每转动一齿触发动作次数
 */
uint8_t EEPROM_GetStepPerTeeth(void);

/**
 * @brief 设置触发动作的次数
 * @param step EC11 编码器每转动一齿触发动作次数
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetStepPerTeeth(uint8_t step);

#endif /* __EEPROM_H__ */
