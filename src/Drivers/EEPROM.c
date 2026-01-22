/*
  EEPROM 配置参数存储驱动实现

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#include "EEPROM.h"

static __xdata eeprom_config_t config;

/**
 * @brief 获取完整的配置结构体数据指针
 * @return 配置结构体指针
 */
inline eeprom_config_t *EEPROM_GetConfigData() { return &config; }

/**
 * @brief 从 EEPROM 读取配置参数
 * @return 操作状态
 */
eeprom_status_t EEPROM_LoadConfig() {
    __xdata uint8_t *data = (__xdata uint8_t *)&config;

    for (uint8_t i = 0; i < CONFIG_STRUCT_SIZE; i++) {
        data[i] = eeprom_read_byte(EEPROM_CONFIG_START_ADDRESS + i);
    }

    // 验证配置数据有效性
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
    // 验证配置数据有效性
    if (EEPROM_Validate() != EEPROM_STATUS_OK) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 写入版本信息默认值
    eeprom_write_byte(EEPROM_CONFIG_START_ADDRESS + 0, FIRMWARE_VERSION);
    eeprom_write_byte(EEPROM_CONFIG_START_ADDRESS + 1, FIRMWARE_REVISION);

    const __xdata uint8_t *data = (const __xdata uint8_t *)&config;

    for (uint8_t i = 2; i < CONFIG_STRUCT_SIZE; i++) {
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
    config.led_count = LED_COUNT_DEFAULT;        // 默认LED数量
    config.color_order = WS2812_COLOR_ORDER_GRB; // 默认颜色顺序
    config.brightness = BRIGHTNESS_DEFAULT;      // 默认亮度等级
    config.effect_mode = EFFECT_MODE_DEFAULT;    // 默认灯效模式
    config.rotate_interval = ROTATE_INTERVAL_DEFAULT; // 默认流动灯效循环周期
    config.fade_duration = FADE_DURATION_DEFAULT; // 默认渐变灯效持续时长
    config.rotate_cw = ROTATE_CW_DEFAULT;         // 默认顺时针旋转角度
    config.rotate_ccw = ROTATE_CCW_DEFAULT;       // 默认逆时针旋转角度
    config.step_per_teeth = STEP_PER_TEETH_DEFAULT; // 默认转动一齿触发次数
    config.phase = EC11_PHASE_A_LEADS; // 默认 EC11 编码器相位配置

    // 初始化预留空间
    memset(config.reserved, 0, sizeof(config.reserved));

    return EEPROM_STATUS_OK;
}

/**
 * @brief 验证配置参数有效性
 * @return 操作状态
 */
eeprom_status_t EEPROM_Validate() {
    // 检查 LED 数量是否在有效范围内
    if (config.led_count < LED_COUNT_MIN || config.led_count > LED_COUNT_MAX) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查颜色顺序是否在有效范围内
    if (config.color_order != WS2812_COLOR_ORDER_GRB &&
        config.color_order != WS2812_COLOR_ORDER_RGB) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查亮度等级是否在有效范围内
    if (config.brightness > BRIGHTNESS_MAX) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查灯效模式是否在有效范围内
    if (config.effect_mode != EFFECT_MODE_DEFAULT) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查流动灯效循环周期是否在有效范围内
    if (config.rotate_interval < ROTATE_INTERVAL_MIN ||
        config.rotate_interval > ROTATE_INTERVAL_MAX) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查渐变灯效持续时长是否在有效范围内
    if (config.fade_duration < FADE_DURATION_MIN ||
        config.fade_duration > FADE_DURATION_MAX) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查顺时针旋转角度是否在有效范围内
    if (config.rotate_cw < ROTATE_ANGLE_MIN ||
        config.rotate_cw > ROTATE_ANGLE_MAX) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查逆时针旋转角度是否在有效范围内
    if (config.rotate_ccw < -ROTATE_ANGLE_MAX ||
        config.rotate_ccw > -ROTATE_ANGLE_MIN) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    // 检查转动一齿触发次数是否在有效范围内
    if (config.step_per_teeth != STEP_PER_TEETH_1X &&
        config.step_per_teeth != STEP_PER_TEETH_2X) {
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
    if (count < LED_COUNT_MIN || count > LED_COUNT_MAX) {
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
 * @return 亮度等级
 */
uint8_t EEPROM_GetBrightness() { return config.brightness; }

/**
 * @brief 设置亮度等级
 * @param brightness 亮度等级
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetBrightness(uint8_t brightness) {
    if (brightness > BRIGHTNESS_MAX) {
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
    if (mode != EFFECT_MODE_DEFAULT) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.effect_mode = mode;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 LED 流动灯效触发间隔
 * @return 触发间隔（毫秒）
 */
uint16_t EEPROM_GetRotateEffectInterval() { return config.rotate_interval; }

/**
 * @brief 设置 LED 流动灯效触发间隔
 * @param interval 触发间隔（毫秒）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetRotateEffectInterval(uint16_t interval) {
    if (interval < ROTATE_INTERVAL_MIN || interval > ROTATE_INTERVAL_MAX) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.rotate_interval = interval;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取 LED 渐变灯效持续时长
 * @return 持续时长（毫秒）
 */
uint16_t EEPROM_GetFadeEffectDuration() { return config.fade_duration; }

/**
 * @brief 设置 LED 渐变灯效持续时长
 * @param duration 持续时长（毫秒）
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetFadeEffectDuration(uint16_t duration) {
    if (duration < FADE_DURATION_MIN || duration > FADE_DURATION_MAX) {
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
    if (degrees < ROTATE_ANGLE_MIN || degrees > ROTATE_ANGLE_MAX) {
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
    if (degrees < -ROTATE_ANGLE_MAX || degrees > -ROTATE_ANGLE_MIN) {
        return EEPROM_STATUS_INVALID_PARAM;
    }

    config.rotate_ccw = degrees;
    return EEPROM_STATUS_OK;
}

/**
 * @brief 获取触发动作次数
 * @return EC11 编码器每转动一齿触发动作次数
 */
uint8_t EEPROM_GetStepPerTeeth() { return config.step_per_teeth; }

/**
 * @brief 设置触发动作次数
 * @param step EC11 编码器每转动一齿触发动作次数
 * @return 操作状态
 */
eeprom_status_t EEPROM_SetStepPerTeeth(uint8_t step) {
    if (step != STEP_PER_TEETH_1X && step != STEP_PER_TEETH_2X) {
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
