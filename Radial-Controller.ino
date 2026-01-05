/*
  Radial Controller

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#ifndef USER_USB_RAM
#error "This example needs to be compiled with a USER USB setting"
#error "Tools--> USB Settings--> USER CODE w/266 USB Ram"
#endif

#include "src/CdcRadial/USBCDC.h"
#include "src/CdcRadial/USBRadial.h"
#include "src/Common.h"
#include "src/Drivers/EC11.h"
#include "src/Drivers/EEPROM.h"
#include "src/Drivers/MyWS2812.h"

void processCommand(char *command);

// 接收缓冲区，用于存储从串口接收的命令
char receive_str[20];
uint8_t receive_ptr = 0;
bool received = false;

// 上一次编码器旋转方向
ec11_direction_t last_direction = EC11_DIR_CW;

// tick 计数器，用于控制 LED 亮度更新频率
uint16_t tick_count = 0;

// 是否为在线配置模式
bool is_config_mode = false;

void setup() {
    USBInit();

    // 从 EEPROM 读取配置参数
    EEPROM_LoadConfig();

    // 初始化 EC11 编码器
    EC11_Init(EC11_PIN_A, EC11_PIN_B, EC11_PIN_KEY);

    // 初始化 WS2812 LED
    WS2812_Init(WS2812_PIN, EEPROM_GetLedCount(), EEPROM_GetColorOrder());
    WS2812_SetBrightness(EEPROM_GetBrightness());
}

void loop() {
    while (USBSerial_available()) {
        char serialChar = USBSerial_read();

        if ((serialChar == '\n') || (serialChar == '\r')) {
            receive_str[receive_ptr] = '\0';
            if (receive_ptr > 0) {
                received = true;
                break;
            }
        } else {
            receive_str[receive_ptr] = serialChar;
            receive_ptr++;

            if (receive_ptr == 63) {
                receive_str[receive_ptr] = '\0';
                received = true;
                break;
            }
        }
    }

    if (received) {
        // 处理接收到的命令
        // DEBUG_PRINT("Command: ");
        // DEBUG_PRINTLN(receive_str);

        processCommand(receive_str);

        received = false;
        receive_ptr = 0;
    }

    if (is_config_mode) {
        delay(1);
        return;
    }

    // 更新 EC11 编码器状态
    EC11_UpdateStatus();

    // 处理编码器旋转
    ec11_direction_t direction = EC11_GetDirection();

    delay(1);
    tick_count++;

    // 定时更新 LED 灯效
    if (tick_count >= EEPROM_GetEffectTick()) {
        tick_count = 0;

        if (direction != EC11_DIR_NONE && direction != last_direction) {
            last_direction = direction;
        }

        // 更新 LED 灯效
        WS2812_SetEffectColor(last_direction);
    }

    if (direction == EC11_DIR_CW) {
        DEBUG_PRINTLN("Encoder rotated right");
        Radial_SendData(0,
                        EEPROM_GetRotateCW()); // 顺时针旋转，发送正值，单位：度
    } else if (direction == EC11_DIR_CCW) {
        DEBUG_PRINTLN("Encoder rotated left");
        Radial_SendData(
            0, EEPROM_GetRotateCCW()); // 逆时针旋转，发送负值，单位：度
    }

    // 处理编码器按键
    if (EC11_IsKeyChanged()) {
        ec11_key_state_t key_state = EC11_GetKeyState();
        if (key_state == EC11_KEY_PRESSED) {
            DEBUG_PRINTLN("Encoder key pressed");
            Radial_SendData(1, 0); // 按键按下
        } else {
            DEBUG_PRINTLN("Encoder key released");
            Radial_SendData(0, 0); // 按键释放
        }
    }
}

/**
 * @brief 处理CDC接收到的命令
 * @param command 接收到的命令字符串
 */
void processCommand(char *command) {
    if (strcmp(command, "config_mode_enabled") == 0) {
        is_config_mode = true;
        USBSerial_println("config_mode_enabled_success");;
        DEBUG_PRINTLN("Config mode enabled");
    } else if (strcmp(command, "config_mode_disabled") == 0) {
        is_config_mode = false;
        DEBUG_PRINTLN("Config mode disabled");
    } else if (strcmp(command, "show_menu") == 0) {
        DEBUG_PRINTLN("Radial button long press");

        // 立即模拟径向控制器按钮按下
        Radial_SendData(1, 0); // button=1(按下), degree=0(无旋转)

        // 保持按下状态0.5秒（500毫秒）
        delay(500);

        // 执行按钮释放动作
        Radial_SendData(0, 0); // button=0(释放), degree=0(无旋转)
    } else if (strcmp(command, "click") == 0) {
        DEBUG_PRINTLN("Radial button click");

        // 立即模拟径向控制器按钮按下
        Radial_SendData(1, 0); // button=1(按下), degree=0(无旋转)

        // 保持按下状态0.1秒（100毫秒）
        delay(100);

        // 执行按钮释放动作
        Radial_SendData(0, 0); // button=0(释放), degree=0(无旋转)
    } else if (strcmp(command, "rotate_l") == 0) {
        DEBUG_PRINTLN("Radial rotate left");

        // 模拟向左旋转（逆时针），单次旋转值为 -10 度
        Radial_SendData(0, -10); // button=0(释放), degree=-10(向左旋转)
    } else if (strcmp(command, "rotate_r") == 0) {
        DEBUG_PRINTLN("Radial rotate right");

        // 模拟向右旋转（顺时针），单次旋转值为 10 度
        Radial_SendData(0, 10); // button=0(释放), degree=10(向右旋转)
    } else if (strcmp(command, "test_led") == 0) {
        DEBUG_PRINTLN("Testing LED");
        // 测试LED灯效
        WS2812_SetAllPixels(255, 0, 0);
        WS2812_Show();
        delay(500);
        WS2812_SetAllPixels(0, 255, 0);
        WS2812_Show();
        delay(500);
        WS2812_SetAllPixels(0, 0, 255);
        WS2812_Show();
        delay(500);
        WS2812_Clear();
        WS2812_Show();
    } else if (strcmp(command, "get_config") == 0) {
        DEBUG_PRINTLN("Getting config");
        // 返回当前配置信息
        USBSerial_print("led_count=");
        USBSerial_println(EEPROM_GetLedCount());
        USBSerial_print("color_order=");
        USBSerial_println(EEPROM_GetColorOrder());
        USBSerial_print("brightness=");
        USBSerial_println(EEPROM_GetBrightness());
        USBSerial_print("effect_mode=");
        USBSerial_println(EEPROM_GetEffectMode());
        USBSerial_print("effect_tick=");
        USBSerial_println(EEPROM_GetEffectTick());
        USBSerial_print("rotate_cw=");
        USBSerial_println(EEPROM_GetRotateCW());
        USBSerial_print("rotate_ccw=");
        USBSerial_println(EEPROM_GetRotateCCW());
    } else if (strncmp(command, "set_", 4) == 0) {
        // 处理set_开头的命令，用于设置单个参数
        char *param = command + 4;
        char *value = strchr(param, '=');
        if (value) {
            *value = '\0';
            value++;

            if (strcmp(param, "led_count") == 0) {
                EEPROM_SetLedCount(atoi(value));
                // 重新初始化WS2812
                WS2812_Init(WS2812_PIN, EEPROM_GetLedCount(),
                            EEPROM_GetColorOrder());
            } else if (strcmp(param, "brightness") == 0) {
                EEPROM_SetBrightness(atoi(value));
                WS2812_SetBrightness(EEPROM_GetBrightness());
            } else if (strcmp(param, "effect_mode") == 0) {
                EEPROM_SetEffectMode(atoi(value));
            } else if (strcmp(param, "effect_tick") == 0) {
                EEPROM_SetEffectTick(atoi(value));
            } else if (strcmp(param, "rotate_cw") == 0) {
                EEPROM_SetRotateCW(atoi(value));
            } else if (strcmp(param, "rotate_ccw") == 0) {
                EEPROM_SetRotateCCW(atoi(value));
            } else if (strcmp(param, "color_order") == 0) {
                EEPROM_SetColorOrder(atoi(value));
                // 重新初始化WS2812
                WS2812_Init(WS2812_PIN, EEPROM_GetLedCount(),
                            EEPROM_GetColorOrder());
            }

            // DEBUG_PRINT("Set ");
            // DEBUG_PRINT(param);
            // DEBUG_PRINT(" to ");
            // DEBUG_PRINTLN(value);
        }
    } else if (strcmp(command, "save_config") == 0) {
        DEBUG_PRINTLN("Saving config");
        EEPROM_SaveConfig();
    } else if (strcmp(command, "reset_config") == 0) {
        DEBUG_PRINTLN("Resetting config");
        EEPROM_Reset();
        // 重新初始化WS2812
        WS2812_Init(WS2812_PIN, EEPROM_GetLedCount(), EEPROM_GetColorOrder());
        WS2812_SetBrightness(EEPROM_GetBrightness());
    }
}
