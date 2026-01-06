/*
  EEPROM 配置参数存储驱动实现

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#include "EEPROM.h"

static config_t config;

/**
 * @brief 获取完整的配置结构体数据
 * @return 配置结构体指针
 */
config_t *EEPROM_GetConfigData(void) { return &config; }

/**
 * @brief 从 EEPROM 读取配置参数
 * @return 操作状态
 */
eeprom_status_t EEPROM_LoadConfig() {
    // 从EEPROM读取配置数据
    for (uint8_t i = 0; i < CONFIG_STRUCT_SIZE; i++) {
        uint8_t *data = (uint8_t *)&config;
        data[i] = eeprom_read_byte(EEPROM_CONFIG_START_ADDRESS + i);
    }

    // 验证配置数据的有效性
    if (EEPROM_Validate() != EEPROM_STATUS_OK) {
        // 配置数据无效，重置为默认值
        if (EEPROM_Reset() == EEPROM_STATUS_OK) {
            // 将默认配置保存到EEPROM
            return EEPROM_SaveConfig();
        }

        return EEPROM_STATUS_ERROR;
    }

    return EEPROM_STATUS_OK;
}

/**
 * @brief 将配置参数写入 EEPROM
 * @return 操作状态
 */
eeprom_status_t EEPROM_SaveConfig() {
    // 验证配置数据的有效性
    if (EEPROM_Validate() != EEPROM_STATUS_OK) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 将配置数据写入EEPROM
    eeprom_write_byte(EEPROM_CONFIG_START_ADDRESS + 0, CURRENT_CONFIG_VERSION);
    eeprom_write_byte(EEPROM_CONFIG_START_ADDRESS + 1, CURRENT_CONFIG_REVISION);

    for (uint8_t i = 2; i < CONFIG_STRUCT_SIZE; i++) {
        const uint8_t *data = (const uint8_t *)&config;
        eeprom_write_byte(EEPROM_CONFIG_START_ADDRESS + i, data[i]);
    }

    return EEPROM_STATUS_OK;
}

/**
 * @brief 重置配置参数为默认值
 * @return 操作状态
 */
eeprom_status_t EEPROM_Reset() {
    // 设置默认配置
    config.version = CURRENT_CONFIG_VERSION;   // 默认版本号
    config.revision = CURRENT_CONFIG_REVISION; // 默认修订号
    config.led_count = 4;                      // 默认LED数量
    config.color_order = 0;                    // 默认颜色顺序（GRB）
    config.brightness = 1;                     // 默认亮度等级
    config.effect_mode = 0;                    // 默认灯效模式
    config.effect_tick = 50; // 默认灯效循环周期（50ms）
    config.rotate_cw = 10;   // 默认顺时针旋转角度
    config.rotate_ccw = -10; // 默认逆时针旋转角度

    // 初始化预留空间为 0
    for (uint8_t i = 0; i < sizeof(config.reserved); i++) {
        config.reserved[i] = 0;
    }

    return EEPROM_STATUS_OK;
}

/**
 * @brief 验证配置参数的有效性
 * @return 操作状态
 */
eeprom_status_t EEPROM_Validate() {
    // 检查亮度等级是否在有效范围内
    if (config.brightness > 5) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查 LED 数量是否在有效范围内（1-10）
    if (config.led_count < 1 || config.led_count > 10) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查颜色顺序是否在有效范围内（0:GRB, 1:RGB）
    if (config.color_order > 1) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查灯效循环周期是否在有效范围内（20-500ms）
    if (config.effect_tick < 20 || config.effect_tick > 500) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查旋转角度是否在有效范围内（1-360度）
    if (config.rotate_cw < 1 || config.rotate_cw > 360 ||
        config.rotate_ccw < -360 || config.rotate_ccw > -1) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取版本号
 * @return 版本号
 */
uint8_t EEPROM_GetVersion(void) { return config.version; }

/**
 * @brief 获取修订号
 * @return 修订号
 */
uint8_t EEPROM_GetRevision(void) { return config.revision; }

/**
 * @brief 获取LED灯珠数量
 * @return LED灯珠数量
 */
uint8_t EEPROM_GetLedCount(void) { return config.led_count; }

/**
 * @brief 设置LED灯珠数量
 * @param count LED灯珠数量
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetLedCount(uint8_t count) {
    // 验证参数有效性
    if (count < 1 || count > 10) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.led_count = count;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取LED颜色顺序
 * @return LED颜色顺序（0:GRB, 1:RGB）
 */
uint8_t EEPROM_GetColorOrder(void) { return config.color_order; }

/**
 * @brief 设置LED颜色顺序
 * @param order LED颜色顺序（0:GRB, 1:RGB）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetColorOrder(uint8_t order) {
    // 验证参数有效性
    if (order > 1) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.color_order = order;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取亮度等级
 * @return 亮度等级（0-4）
 */
uint8_t EEPROM_GetBrightness(void) { return config.brightness; }

/**
 * @brief 设置亮度等级
 * @param brightness 亮度等级（0-4）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetBrightness(uint8_t brightness) {
    // 验证参数有效性
    if (brightness > 4) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.brightness = brightness;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 LED 灯效模式
 * @return LED 灯效模式
 */
uint8_t EEPROM_GetEffectMode(void) { return config.effect_mode; }

/**
 * @brief 设置 LED 灯效模式
 * @param mode LED 灯效模式
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetEffectMode(uint8_t mode) {
    config.effect_mode = mode;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 LED 灯效循环周期
 * @return LED 灯效循环周期（ms）
 */
uint16_t EEPROM_GetEffectTick(void) { return config.effect_tick; }

/**
 * @brief 设置 LED 灯效循环周期
 * @param tick LED 灯效循环周期（ms）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetEffectTick(uint16_t tick) {
    // 验证参数有效性
    if (tick < 20 || tick > 500) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.effect_tick = tick;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 LED 顺时针旋转角度
 * @return 顺时针旋转角度
 */
int16_t EEPROM_GetRotateCW(void) { return config.rotate_cw; }

/**
 * @brief 设置 LED 顺时针旋转角度
 * @param degrees 顺时针旋转角度
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetRotateCW(int16_t degrees) {
    // 验证参数有效性
    if (degrees < 1 || degrees > 360) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.rotate_cw = degrees;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 LED 逆时针旋转角度
 * @return 逆时针旋转角度
 */
int16_t EEPROM_GetRotateCCW(void) { return config.rotate_ccw; }

/**
 * @brief 设置 LED 逆时针旋转角度
 * @param degrees 逆时针旋转角度
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetRotateCCW(int16_t degrees) {
    // 验证参数有效性
    if (degrees < -360 || degrees > -1) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.rotate_ccw = degrees;
    return EEPROM_STATUS_OK;
}
