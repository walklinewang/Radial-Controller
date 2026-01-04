/*
  径向控制器头文件

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
#ifndef __USB_RADIAL_H__
#define __USB_RADIAL_H__

// clang-format off
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>
#include "include/ch5xx.h"
#include "include/ch5xx_usb.h"
// clang-format on

// 径向控制器报告结构定义
#define RADIAL_REPORT_ID 0x01
#define RADIAL_REPORT_SIZE 3 // 报告总大小(字节): reportId(1) + buttonDial(2)

// 错误代码定义
#define HID_ERR_NONE 0               // 无错误
#define HID_ERR_USB_NOT_CONFIGURED 1 // USB未配置
#define HID_ERR_BUFFER_BUSY 2        // 发送缓冲区忙
#define HID_ERR_INVALID_PARAM 3      // 参数无效
#define HID_ERR_DATA_TOO_LONG 4      // 数据过长

// 径向控制器数据结构
typedef struct {
    uint8_t reportId;    // 报告ID (固定为1)
    uint16_t buttonDial; // 按钮(bit0)和旋钮(bit1-15)的组合字节
} RadialReport;

// 按钮状态位掩码
#define RADIAL_BUTTON_MASK 0x0001
// 旋钮值位掩码
#define RADIAL_DIAL_MASK 0xFFFE
// 旋钮值符号位掩码
#define RADIAL_DIAL_SIGN_MASK 0x8000

// CH5xx USB端点3接收长度寄存器定义
#ifndef UEP3_RX_LEN
#define UEP3_RX_LEN (*(volatile uint8_t __xdata *)0x9E)
#endif

#ifdef __cplusplus
extern "C" {
#endif

/**
 * @brief 发送径向控制器报告
 * @param report 径向控制器报告指针
 * @return bool 发送成功返回 true，失败返回 false
 */
bool Radial_SendReport(__xdata RadialReport *report);

/**
 * @brief 发送径向控制器数据
 * @param button 按钮状态 (0=释放, 1=按下)
 * @param degree 旋钮角度 (-360~360)
 * @return bool 发送成功返回 true，失败返回 false
 */
bool Radial_SendData(__data uint8_t button, __data int16_t degree);

/**
 * @brief 重置径向控制器报告
 */
void Radial_ResetReport(void);

/**
 * @brief 获取径向控制器报告指针
 * @return RadialReport* 径向控制器报告指针
 */
RadialReport *Radial_GetReport(void);

/**
 * @brief 获取最后一个错误代码
 * @return uint8_t 错误代码
 */
uint8_t Radial_GetLastError(void);

#ifdef __cplusplus
} // extern "C"
#endif

#endif /* __USB_RADIAL_H__ */
