/*
  EEPROM 配置参数存储驱动实现

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#include "EEPROM.h"

static eeprom_config_t config;

/**
 * @brief 获取完整的配置结构体数据
 * @return 配置结构体指针
 */
eeprom_config_t *EEPROM_GetConfigData() { return &config; }

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

    // 版本信息写入默认值
    eeprom_write_byte(EEPROM_CONFIG_START_ADDRESS + 0, FIRMWARE_VERSION);
    eeprom_write_byte(EEPROM_CONFIG_START_ADDRESS + 1, FIRMWARE_REVISION);

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
    config.version = FIRMWARE_VERSION;           // 默认版本号
    config.revision = FIRMWARE_REVISION;         // 默认修订号
    config.led_count = 4;                        // 默认LED数量
    config.color_order = WS2812_COLOR_ORDER_GRB; // 默认颜色顺序（GRB）
    config.brightness = 3;                       // 默认亮度等级
    config.effect_mode = 0;                      // 默认灯效模式
    config.rotate_interval = 50; // 默认流动灯效循环周期（50ms）
    config.fade_duration = 100;  // 默认渐变灯效持续时间（300ms）
    config.rotate_cw = 10;       // 默认顺时针旋转角度
    config.rotate_ccw = -10;     // 默认逆时针旋转角度
    config.step_per_teeth = 2;   // 默认转动一齿触发次数
    config.phase = EC11_PHASE_A_LEADS; // 默认 EC11 编码器相位配置

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
    // 检查 LED 数量是否在有效范围内
    if (config.led_count < 1 || config.led_count > 10) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查颜色顺序是否在有效范围内
    if (config.color_order != WS2812_COLOR_ORDER_GRB &&
        config.color_order != WS2812_COLOR_ORDER_RGB) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查亮度等级是否在有效范围内
    if (config.brightness > 5) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查灯效模式是否在有效范围内
    if (config.effect_mode != 0) { // 简单范围检查，实际可能需要更严格的限制
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查流动灯效循环周期是否在有效范围内
    if (config.rotate_interval < 20 || config.rotate_interval > 500) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查渐变灯效持续时间是否在有效范围内
    if (config.fade_duration < 20 || config.fade_duration > 500) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查顺时针旋转角度是否在有效范围内
    if (config.rotate_cw < 1 || config.rotate_cw > 360) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查逆时针旋转角度是否在有效范围内
    if (config.rotate_ccw < -360 || config.rotate_ccw > -1) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查转动一齿触发次数是否在有效范围内
    if (config.step_per_teeth != 1 && config.step_per_teeth != 2) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查 EC11 编码器相位配置是否在有效范围内
    if (config.phase != EC11_PHASE_A_LEADS &&
        config.phase != EC11_PHASE_B_LEADS) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取版本号
 * @return 版本号
 */
uint8_t EEPROM_GetVersion() { return config.version; }

/**
 * @brief 获取修订号
 * @return 修订号
 */
uint8_t EEPROM_GetRevision() { return config.revision; }

/**
 * @brief 获取LED灯珠数量
 * @return LED灯珠数量
 */
uint8_t EEPROM_GetLedCount() { return config.led_count; }

/**
 * @brief 设置LED灯珠数量
 * @param count LED灯珠数量
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetLedCount(uint8_t count) {
    if (count < 1 || count > 10) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.led_count = count;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取LED颜色顺序
 * @return LED颜色顺序
 */
ws2812_color_order_t EEPROM_GetColorOrder() { return config.color_order; }

/**
 * @brief 设置LED颜色顺序
 * @param order LED颜色顺序
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetColorOrder(ws2812_color_order_t order) {
    if (order != WS2812_COLOR_ORDER_GRB && order != WS2812_COLOR_ORDER_RGB) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.color_order = order;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取亮度等级
 * @return 亮度等级（0-4）
 */
uint8_t EEPROM_GetBrightness() { return config.brightness; }

/**
 * @brief 设置亮度等级
 * @param brightness 亮度等级（0-4）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetBrightness(uint8_t brightness) {
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
uint8_t EEPROM_GetEffectMode() { return config.effect_mode; }

/**
 * @brief 设置 LED 灯效模式
 * @param mode LED 灯效模式
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetEffectMode(uint8_t mode) {
    if (mode != 1) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.effect_mode = mode;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 LED 流动灯效循环周期
 * @return LED 流动灯效循环周期（ms）
 */
uint16_t EEPROM_GetRotateEffectInterval() { return config.rotate_interval; }

/**
 * @brief 设置 LED 流动灯效循环周期
 * @param interval LED 流动灯效循环周期（ms）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetRotateEffectInterval(uint16_t interval) {
    if (interval < 20 || interval > 500) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.rotate_interval = interval;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 LED 渐变灯效持续时间
 * @return LED 渐变灯效持续时间（ms）
 */
uint16_t EEPROM_GetFadeEffectDuration() { return config.fade_duration; }

/**
 * @brief 设置 LED 渐变灯效持续时间
 * @param duration LED 渐变灯效持续时间（ms）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetFadeEffectDuration(uint16_t duration) {
    if (duration < 20 || duration > 500) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.fade_duration = duration;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 LED 顺时针旋转角度
 * @return 顺时针旋转角度
 */
int16_t EEPROM_GetRotateCW() { return config.rotate_cw; }

/**
 * @brief 设置 LED 顺时针旋转角度
 * @param degrees 顺时针旋转角度
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetRotateCW(int16_t degrees) {
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
int16_t EEPROM_GetRotateCCW() { return config.rotate_ccw; }

/**
 * @brief 设置 LED 逆时针旋转角度
 * @param degrees 逆时针旋转角度
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetRotateCCW(int16_t degrees) {
    if (degrees < -360 || degrees > -1) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.rotate_ccw = degrees;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取触发动作的次数
 * @return EC11 编码器每转动一齿触发动作次数
 */
uint8_t EEPROM_GetStepPerTeeth() { return config.step_per_teeth; }

/**
 * @brief 设置触发动作的次数
 * @param step EC11 编码器每转动一齿触发动作次数
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetStepPerTeeth(uint8_t step) {
    if (step != 1 && step != 2) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.step_per_teeth = step;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 EC11 编码器相位配置
 * @return EC11 编码器相位配置
 */
ec11_phase_t EEPROM_GetPhase() { return config.phase; }

/**
 * @brief 设置 EC11 编码器相位配置
 * @param phase EC11 编码器相位配置
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetPhase(ec11_phase_t phase) {
    if (phase != EC11_PHASE_A_LEADS && phase != EC11_PHASE_B_LEADS) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.phase = phase;
    return EEPROM_STATUS_OK;
}
