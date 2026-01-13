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

#define CMD_CONFIG_MODE_ENABLED "config_mode_enabled"
#define CMD_CONFIG_SAVE_SETTINGS "save_settings"
#define CMD_CONFIG_RESET_SETTINGS "reset_settings"
#define CMD_SUCCESS "_success"
#define CMD_FAILED "_failed"

#define HEARTBEAT_TIMEOUT 4000 // 心跳超时时间（4秒）

void update_config();
void process_heartbeat();
void process_command(uint8_t *command);

// 接收缓冲区，用于存储从串口接收的命令
uint8_t receive_buf[50];
uint8_t receive_ptr = 0;
bool received = false;

// 上一次编码器旋转方向
ec11_direction_t last_direction = EC11_DIR_CW;

// 是否为配置模式
bool is_config_mode = false;

// 心跳检测相关变量
uint32_t heartbeat_last_received = 0; // 最后一次收到心跳的时间戳

void setup() {
    USBInit();

    // 从 EEPROM 读取配置参数
    EEPROM_LoadConfig();

    // 初始化 EC11 编码器
    EC11_Init(EC11_PIN_A, EC11_PIN_B, EC11_PIN_K);

    // 执行配置初始化
    update_config();
}

void loop() {
    bool is_save_command = false;

    while (USBSerial_available()) {
        uint8_t serial_char = USBSerial_read();

        // 针对 save_settings 命令的特殊处理
        if (receive_ptr >= 14) {
            is_save_command = (memcmp(receive_buf, "save_settings=", 14) == 0);
        }

        if (is_save_command) {
            receive_buf[receive_ptr] = serial_char;
            receive_ptr++;

            // save_settings=命令格式：14字节前缀 + 30字节数据 + 1字节换行符
            if (receive_ptr >= 14 + 30 + 1) {
                receive_buf[receive_ptr] = '\0';
                received = true;
                break;
            }
        }
        // 对于其他命令，使用换行符或回车符作为结束标记
        else if ((serial_char == '\n') || (serial_char == '\r')) {
            receive_buf[receive_ptr] = '\0';
            if (receive_ptr > 0) {
                received = true;
                break;
            }
        } else {
            receive_buf[receive_ptr] = serial_char;
            receive_ptr++;

            if (receive_ptr >= sizeof(receive_buf) - 1) {
                receive_buf[receive_ptr] = '\0';
                received = true;
                break;
            }
        }
    }

    if (received) {
        process_command(receive_buf);

        receive_ptr = 0;
        received = false;
    }

    if (is_config_mode) {
        process_heartbeat();

        delay(1);
        return;
    }

    // 更新 EC11 编码器状态
    EC11_UpdateStatus();

    // 处理编码器旋转
    ec11_direction_t direction = EC11_GetDirection();

    delay(1);

    // 更新方向状态
    if (direction != EC11_DIR_NONE && direction != last_direction) {
        last_direction = direction;
    }

    // 只有在流动灯效状态下才更新 LED 流动灯效
    if (WS2812_GetEffectState() == WS2812_EFFECT_STATE_ROTATION) {
        WS2812_ShowRotationEffect(last_direction);
    }

    if (direction == EC11_DIR_CW) {
        Radial_SendData(0,
                        EEPROM_GetRotateCW()); // 顺时针旋转，发送正值，单位：度
    } else if (direction == EC11_DIR_CCW) {
        Radial_SendData(
            0, EEPROM_GetRotateCCW()); // 逆时针旋转，发送负值，单位：度
    }

    // 执行 Fade 效果
    WS2812_ShowFadeEffect();

    // 处理编码器按键
    if (EC11_IsKeyChanged()) {
        ec11_key_state_t key_state = EC11_GetKeyState();

        if (key_state == EC11_KEY_PRESSED) {
            Radial_SendData(1, 0);     // 按键按下
            WS2812_SetFadeOutEffect(); // 渐暗效果
        } else {
            Radial_SendData(0, 0);    // 按键释放
            WS2812_SetFadeInEffect(); // 渐亮效果
        }
    }
}

/**
 * @brief 配置更新后执行的初始化操作
 */
void update_config() {
    // 设置 EC11 编码器转动一齿触发次数
    EC11_SetStepPerTeeth(EEPROM_GetStepPerTeeth());

    // 重新初始化 WS2812 LED
    WS2812_Init(WS2812_PIN, EEPROM_GetLedCount(), EEPROM_GetColorOrder());
    WS2812_SetBrightness(EEPROM_GetBrightness());

    // 设置 LED 流动灯效的触发间隔时间
    WS2812_SetRotateEffectInterval(EEPROM_GetRotateEffectInterval());

    // 设置 LED 渐变灯效的持续时间
    WS2812_SetFadeEffectDuration(EEPROM_GetFadeEffectDuration());
}

/**
 * @brief 处理心跳包命令
 */
void process_heartbeat() {
    if (millis() - heartbeat_last_received >= HEARTBEAT_TIMEOUT) {
        // 心跳超时，退出配置模式
        is_config_mode = false;
        heartbeat_last_received = 0; // 重置心跳时间戳

        USBSerial_println("config_mode_timedout");
        USBSerial_flush();
    }
}

/**
 * @brief 处理 CDC 接收到的命令
 * @param command 接收到的命令字符串
 */
void process_command(uint8_t *command) {
    if (strcmp((const uint8_t *)command, CMD_CONFIG_MODE_ENABLED) == 0) {
        is_config_mode = true;

        // 初始化心跳检测，设置最后收到心跳时间为当前时间
        heartbeat_last_received = millis();

        USBSerial_print(CMD_CONFIG_MODE_ENABLED);
        USBSerial_println(CMD_SUCCESS);
        USBSerial_flush();
    } else if (strcmp((const uint8_t *)command, "heartbeat") == 0) {
        // 接收网页端发送的心跳包
        if (is_config_mode) {
            heartbeat_last_received = millis(); // 更新最后收到心跳的时间戳
        }
    } else if (strcmp((const uint8_t *)command, "load_settings") == 0) {
        // 从 EEPROM 加载配置参数
        EEPROM_LoadConfig();

        // 获取完整配置结构体
        eeprom_config_t *config = EEPROM_GetConfigData();

        // 发送配置数据前缀
        USBSerial_print("config=");

        // 发送 32 字节配置数据
        for (uint8_t i = 0; i < CONFIG_STRUCT_SIZE; i++) {
            const uint8_t *data = (const uint8_t *)config;
            USBSerial_write(data[i]);
        }

        USBSerial_println();
        USBSerial_flush();
    } else if (memcmp((const uint8_t *)command, "save_settings=", 14) == 0) {
        // 从命令中提取配置数据
        eeprom_config_t *config = EEPROM_GetConfigData();

        // 跳过"save_settings="前缀（14字节）
        const uint8_t *data_ptr = (const uint8_t *)(command + 14);

        uint8_t *config_bytes = (uint8_t *)config;

        // 复制 30 字节配置数据，跳过前 2 字节（version 和 revision）
        for (uint8_t i = 0; i < 30; i++) {
            config_bytes[2 + i] = data_ptr[i];
        }

        // 保存配置到EEPROM
        if (EEPROM_SaveConfig() == EEPROM_STATUS_OK) {
            // 执行配置更新后的初始化操作
            update_config();

            USBSerial_print(CMD_CONFIG_SAVE_SETTINGS);
            USBSerial_println(CMD_SUCCESS);
            USBSerial_flush();
        } else {
            USBSerial_print(CMD_CONFIG_SAVE_SETTINGS);
            USBSerial_println(CMD_FAILED);
            USBSerial_flush();
        }
    } else if (strcmp((const uint8_t *)command, CMD_CONFIG_RESET_SETTINGS) ==
               0) {
        EEPROM_Reset();
        EEPROM_SaveConfig();

        // 执行配置更新后的初始化操作
        update_config();

        USBSerial_print(CMD_CONFIG_RESET_SETTINGS);
        USBSerial_println(CMD_SUCCESS);
        USBSerial_flush();

        /* 以下为测试用命令 */
    } else if (strcmp((const uint8_t *)command, "show_menu") == 0) {
        // 模拟径向控制器按钮按下
        Radial_SendData(1, 0); // button=1(按下), degree=0(无旋转)

        // 保持按下状态 500 毫秒
        delay(500);

        // 执行按钮释放动作
        Radial_SendData(0, 0); // button=0(释放), degree=0(无旋转)
    } else if (strcmp((const uint8_t *)command, "click") == 0) {
        // 立即模拟径向控制器按钮按下
        Radial_SendData(1, 0); // button=1(按下), degree=0(无旋转)

        // 保持按下状态 100 毫秒
        delay(100);

        // 执行按钮释放动作
        Radial_SendData(0, 0); // button=0(释放), degree=0(无旋转)
    } else if (strcmp((const uint8_t *)command, "rotate_l") == 0) {
        // 模拟向左旋转（逆时针），单次旋转值为 -10 度
        Radial_SendData(0, -10); // button=0(释放), degree=-10(向左旋转)
    } else if (strcmp((const uint8_t *)command, "rotate_r") == 0) {
        // 模拟向右旋转（顺时针），单次旋转值为 10 度
        Radial_SendData(0, 10); // button=0(释放), degree=10(向右旋转)
    } else if (strcmp((const uint8_t *)command, "test_led") == 0) {
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
    }
}
