#ifndef __COMMON_H__
#define __COMMON_H__

/* 固件版本号和修订号 */
#define FIRMWARE_VERSION 0
#define FIRMWARE_REVISION 3

/* EC11 编码器引脚定义 */
#define EC11_PIN_A 33
#define EC11_PIN_B 31
#define EC11_PIN_K 32

/* WS2812 引脚定义 */
#define WS2812_PIN 15

/*********************
 * EEPROM 配置参数定义 *
 *********************/

// clang-format off
/* LED 数量配置 */
#define LED_COUNT_MIN     1 // LED 最小数量
#define LED_COUNT_MAX    10 // LED 最大数量
#define LED_COUNT_DEFAULT 4 // LED 默认数量

/* 亮度等级配置 */
#define BRIGHTNESS_MIN     0 // 最小亮度等级
#define BRIGHTNESS_MAX     4 // 最大亮度等级
#define BRIGHTNESS_DEFAULT 3 // 默认亮度等级

/* 灯效模式配置 */
#define EFFECT_MODE_DEFAULT 0 // 默认灯效模式

/* 流动灯效循环周期配置 */
#define ROTATE_INTERVAL_MIN     20 // 最小循环周期
#define ROTATE_INTERVAL_MAX    500 // 最大循环周期
#define ROTATE_INTERVAL_DEFAULT 40 // 默认循环周期

/* 渐变灯效持续时间配置 */
#define FADE_DURATION_MIN     100 // 最小持续时间
#define FADE_DURATION_MAX     300 // 最大持续时间
#define FADE_DURATION_DEFAULT 150 // 默认持续时间

/* 旋转角度配置 (度) */
#define ROTATE_ANGLE_MIN     1 // 最小旋转角度
#define ROTATE_ANGLE_MAX   360 // 最大旋转角度
#define ROTATE_CW_DEFAULT   10 // 默认顺时针旋转角度
#define ROTATE_CCW_DEFAULT -10 // 默认逆时针旋转角度

/* 旋转触发次数配置 */
#define STEP_PER_TEETH_1X           1 // 转动一齿触发1次
#define STEP_PER_TEETH_2X           2 // 转动一齿触发2次
#define STEP_PER_TEETH_DEFAULT      2 // 默认触发次数
#define STEP_PER_TEETH_1X_THRESHOLD 2 // 1 次触发阈值
#define STEP_PER_TEETH_2X_THRESHOLD 1 // 2 次触发阈值
// clang-format on

#endif /* __COMMON_H__ */
