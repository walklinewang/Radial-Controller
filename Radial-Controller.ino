/*
  Radial Controller

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#ifndef USER_USB_RAM
#error "This firmware needs to be compiled with a USER USB setting"
#error "Tools--> USB Settings--> USER CODE w/266 USB Ram"
#endif

#include "src/CdcRadial/USBCDC.h"
#include "src/CdcRadial/USBRadial.h"
#include "src/Common.h"
#include "src/Drivers/EC11.h"
#include "src/Drivers/EEPROM.h"
#include "src/Drivers/MyWS2812.h"

#define CMD_CONFIG_MODE_ENABLED "config_mode_enabled"
#define CMD_CONFIG_MODE_TIMEOUT "config_mode_timeout"
#define CMD_CONFIG_LOAD_SETTINGS "load_settings"
#define CMD_CONFIG_SAVE_SETTINGS "save_settings"
#define CMD_CONFIG_RESET_SETTINGS "reset_settings"
#define CMD_CONFIG_HEARTBEAT "heartbeat"
#define CMD_CONFIG_SAVE_SETTINGS_PREFIX CMD_CONFIG_SAVE_SETTINGS "="
#define CMD_SUCCESS_SUFFIX "_success"
#define CMD_FAILED_SUFFIX "_failed"

#define CMD_TEST_SHOW_MENU "show_menu"
#define CMD_TEST_CLICK "click"
#define CMD_TEST_ROTATE_LEFT "rotate_left"
#define CMD_TEST_ROTATE_RIGHT "rotate_right"

#define HEARTBEAT_TIMEOUT 4000 // 心跳超时时间

void update_config();
void process_ec11_operation();
void process_heartbeat();
void process_commands(uint8_t *command);

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
        if (receive_ptr >= strlen(CMD_CONFIG_SAVE_SETTINGS_PREFIX)) {
            is_save_command =
                (memcmp(receive_buf, CMD_CONFIG_SAVE_SETTINGS_PREFIX,
                        strlen(CMD_CONFIG_SAVE_SETTINGS_PREFIX)) == 0);
        }

        if (is_save_command) {
            receive_buf[receive_ptr] = serial_char;
            receive_ptr++;

            // save_settings=命令格式：14字节前缀 + 30字节数据 + 1字节换行符
            if (receive_ptr >=
                strlen(CMD_CONFIG_SAVE_SETTINGS_PREFIX) + 30 + 1) {
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
        process_commands(receive_buf);

        receive_ptr = 0;
        received = false;
    }

    if (is_config_mode) {
        process_heartbeat();
        return;
    }

    process_ec11_operation();

    // 根据当前灯效状态执行对应的灯效
    if (WS2812_GetEffectState() == WS2812_EFFECT_STATE_ROTATION) {
        WS2812_ShowRotationEffect(last_direction);
    } else {
        WS2812_ShowFadeEffect();
    }
}

/**
 * @brief 配置更新后执行的初始化操作
 */
void update_config() {
    // 设置 EC11 编码器转动一齿触发次数
    EC11_SetStepPerTeeth(EEPROM_GetStepPerTeeth());

    // 设置 EC11 编码器相位
    EC11_SetPhase(EEPROM_GetPhase());

    // 重新初始化 WS2812 LED
    WS2812_Init(WS2812_PIN, EEPROM_GetLedCount(), EEPROM_GetColorOrder());

    // 设置 WS2812 LED 亮度
    WS2812_SetBrightness(EEPROM_GetBrightness());

    // 设置 LED 流动灯效的触发间隔时间
    WS2812_SetRotateEffectInterval(EEPROM_GetRotateEffectInterval());

    // 设置 LED 渐变灯效的持续时间
    WS2812_SetFadeEffectDuration(EEPROM_GetFadeEffectDuration());
}

void process_ec11_operation() {
    // 更新 EC11 编码器状态
    EC11_UpdateStatus();

    // 处理编码器旋转
    ec11_direction_t direction = EC11_GetDirection();

    // 更新方向状态
    if (direction != EC11_DIR_NONE && direction != last_direction) {
        last_direction = direction;
    }

    if (direction == EC11_DIR_CW) {
        // 顺时针旋转，发送正值，单位：度
        Radial_SendData(0, EEPROM_GetRotateCW());
    } else if (direction == EC11_DIR_CCW) {
        // 逆时针旋转，发送负值，单位：度
        Radial_SendData(0, EEPROM_GetRotateCCW());
    }

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
 * @brief 处理心跳包命令
 */
void process_heartbeat() {
    if (millis() - heartbeat_last_received >= HEARTBEAT_TIMEOUT) {
        // 心跳超时，退出配置模式
        is_config_mode = false;
        heartbeat_last_received = 0; // 重置心跳时间戳

        USBSerial_println(CMD_CONFIG_MODE_TIMEOUT);
        USBSerial_flush();
    }
}

/**
 * @brief 处理 CDC 接收到的命令
 * @param command 接收到的命令字符串
 */
void process_commands(uint8_t *command) {
    if (strcmp((const uint8_t *)command, CMD_CONFIG_MODE_ENABLED) == 0) {
        is_config_mode = true;

        // 初始化心跳检测，设置最后收到心跳时间为当前时间
        heartbeat_last_received = millis();

        USBSerial_print(CMD_CONFIG_MODE_ENABLED);
        USBSerial_println(CMD_SUCCESS_SUFFIX);
        USBSerial_flush();
    } else if (strcmp((const uint8_t *)command, CMD_CONFIG_HEARTBEAT) == 0) {
        // 接收网页端发送的心跳包
        if (is_config_mode) {
            heartbeat_last_received = millis(); // 更新最后收到心跳的时间戳
        }
    } else if (strcmp((const uint8_t *)command, CMD_CONFIG_LOAD_SETTINGS) ==
               0) {
        // 从 EEPROM 加载配置参数
        EEPROM_LoadConfig();

        eeprom_config_t *config = EEPROM_GetConfigData();
        uint8_t *config_bytes = (uint8_t *)config;

        // 发送配置数据前缀
        USBSerial_print("config=");
        // 发送 32 字节配置数据
        USBSerial_print_n(config_bytes, CONFIG_STRUCT_SIZE);

        USBSerial_println();
        USBSerial_flush();
    } else if (memcmp((const uint8_t *)command, CMD_CONFIG_SAVE_SETTINGS_PREFIX,
                      strlen(CMD_CONFIG_SAVE_SETTINGS_PREFIX)) == 0) {
        eeprom_config_t *config = EEPROM_GetConfigData();

        // 跳过"save_settings="前缀（14字节）
        const uint8_t *data_ptr =
            (const uint8_t *)(command +
                              strlen(CMD_CONFIG_SAVE_SETTINGS_PREFIX));
        uint8_t *config_bytes = (uint8_t *)config;

        // 复制 30 字节配置数据，跳过前 2 字节（version 和 revision）
        memcpy(config_bytes + 2, data_ptr, 30);

        // 保存配置到EEPROM
        if (EEPROM_SaveConfig() == EEPROM_STATUS_OK) {
            // 执行配置更新后的初始化操作
            update_config();

            USBSerial_print(CMD_CONFIG_SAVE_SETTINGS);
            USBSerial_println(CMD_SUCCESS_SUFFIX);
            USBSerial_flush();
        } else {
            // 保存失败，从 EEPROM 加载配置参数
            EEPROM_LoadConfig();

            USBSerial_print(CMD_CONFIG_SAVE_SETTINGS);
            USBSerial_println(CMD_FAILED_SUFFIX);
            USBSerial_flush();
        }
    } else if (strcmp((const uint8_t *)command, CMD_CONFIG_RESET_SETTINGS) ==
               0) {
        EEPROM_Reset();
        EEPROM_SaveConfig();

        // 执行配置更新后的初始化操作
        update_config();

        USBSerial_print(CMD_CONFIG_RESET_SETTINGS);
        USBSerial_println(CMD_SUCCESS_SUFFIX);
        USBSerial_flush();

        /* 以下为测试用命令 */
    } else if (strcmp((const uint8_t *)command, CMD_TEST_SHOW_MENU) == 0) {
        // 模拟径向控制器按钮按下
        Radial_SendData(1, 0); // button=1(按下), degree=0(无旋转)

        // 保持按下状态 500 毫秒
        delay(500);

        // 执行按钮释放动作
        Radial_SendData(0, 0); // button=0(释放), degree=0(无旋转)
    } else if (strcmp((const uint8_t *)command, CMD_TEST_CLICK) == 0) {
        // 立即模拟径向控制器按钮按下
        Radial_SendData(1, 0); // button=1(按下), degree=0(无旋转)

        // 保持按下状态 100 毫秒
        delay(50);

        // 执行按钮释放动作
        Radial_SendData(0, 0); // button=0(释放), degree=0(无旋转)
    } else if (strcmp((const uint8_t *)command, CMD_TEST_ROTATE_LEFT) == 0) {
        // 模拟向左旋转（逆时针），单次旋转值为 -10 度
        Radial_SendData(0, -10); // button=0(释放), degree=-10(向左旋转)
    } else if (strcmp((const uint8_t *)command, CMD_TEST_ROTATE_RIGHT) == 0) {
        // 模拟向右旋转（顺时针），单次旋转值为 10 度
        Radial_SendData(0, 10); // button=0(释放), degree=10(向右旋转)
    }
}
