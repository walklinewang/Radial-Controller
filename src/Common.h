#ifndef __COMMON_H__
#define __COMMON_H__

#define DEBUG 1

// EC11 编码器引脚定义
#define EC11_PIN_A 12
#define EC11_PIN_B 13
#define EC11_PIN_KEY 15

// WS2812 引脚定义
#define WS2812_PIN 16

#if DEBUG
#define DEBUG_PRINT(...) USBSerial_print(__VA_ARGS__)
#define DEBUG_PRINTLN(...)                                                     \
    do {                                                                       \
        USBSerial_println(__VA_ARGS__);                                        \
        USBSerial_flush();                                                     \
    } while (0)
#else
#define DEBUG_PRINT(...)
#define DEBUG_PRINTLN(...)
#endif

#endif /* __COMMON_H__ */
