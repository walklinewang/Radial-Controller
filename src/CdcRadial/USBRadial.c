/*
  径向控制器源文件

  Copyright © 2026 Walkline Wang (walkline@gmail.com)
  Github: https://github.com/walklinewang/Radial-Controller
*/
// clang-format off
#include <stdint.h>
#include <stdbool.h>
#include "include/ch5xx.h"
#include "include/ch5xx_usb.h"
#include "USBconstant.h"
#include "USBhandler.h"
#include "USBRadial.h"
// clang-format on

// clang-format off
extern __xdata __at (EP0_ADDR) uint8_t Ep0Buffer[16];
extern __xdata __at (EP1_ADDR) uint8_t Ep1Buffer[8];
extern __xdata __at (EP2_ADDR) uint8_t Ep2Buffer[128];
extern __xdata __at (EP3_ADDR) uint8_t Ep3Buffer[64];
// clang-format on

volatile __xdata uint8_t UpPoint3_Busy =
    0; // Flag of whether upload pointer is busy

// 径向控制器报告全局变量
__xdata RadialReport radialReport = {.reportId = RADIAL_REPORT_ID,
                                     .buttonDial = 0};

// 错误代码全局变量
__xdata uint8_t lastError = HID_ERR_NONE;

typedef void (*pTaskFn)(void);

void delayMicroseconds(uint16_t us);

void USB_EP3_IN() {
    UEP3_T_LEN = 0;
    UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_NAK; // Default NAK
    UpPoint3_Busy = 0;                                       // Clear busy flag
}

/**
 * @brief USB端点3 OUT事件处理函数，用于接收径向控制器数据
 */
void USB_EP3_OUT() {
    if (UEP3_RX_LEN > 0) {
        // 检查报告 ID
        if (Ep3Buffer[0] == RADIAL_REPORT_ID) {
            // 确保接收到完整的报告数据
            if (UEP3_RX_LEN >= RADIAL_REPORT_SIZE) {
                // 复制接收到的数据到径向控制器报告结构
                __xdata uint8_t *reportPtr = (__xdata uint8_t *)&radialReport;
                for (__data uint8_t i = 0; i < RADIAL_REPORT_SIZE; i++) {
                    reportPtr[i] = Ep3Buffer[i];
                }
            }
        }
    }

    // 清空接收长度并设置为 NAK 状态
    UEP3_RX_LEN = 0;
    UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_R_RES | UEP_R_RES_NAK;
}

/**
 * @brief 发送径向控制器报告
 * @param report 径向控制器报告指针
 * @return bool 发送成功返回 true，失败返回 false
 */
bool Radial_SendReport(__xdata RadialReport *report) {
    // 参数验证
    if (report == NULL) {
        lastError = HID_ERR_INVALID_PARAM;
        return false;
    }

    // 检查USB是否已配置
    if (UsbConfig == 0) {
        lastError = HID_ERR_USB_NOT_CONFIGURED;
        return false;
    }

    __data uint16_t waitWriteCount = 0;

    // 等待发送缓冲区空闲
    waitWriteCount = 0;
    while (UpPoint3_Busy) { // 等待 250ms 或放弃
        waitWriteCount++;
        delayMicroseconds(5);
        if (waitWriteCount >= 50000) {
            lastError = HID_ERR_BUFFER_BUSY;
            return false;
        }
    }

    // 确保报告 ID 正确
    report->reportId = RADIAL_REPORT_ID;

    // 将报告数据加载到发送缓冲区
    __xdata uint8_t *reportPtr = (__xdata uint8_t *)report;
    for (__data uint8_t i = 0; i < RADIAL_REPORT_SIZE; i++) {
        Ep3Buffer[i] = reportPtr[i];
    }

    // 设置发送长度并启动发送
    UEP3_T_LEN = RADIAL_REPORT_SIZE;
    UpPoint3_Busy = 1;
    UEP3_CTRL = UEP3_CTRL & ~MASK_UEP_T_RES | UEP_T_RES_ACK;

    // 发送成功
    lastError = HID_ERR_NONE;
    return true;
}

/**
 * @brief 发送径向控制器数据
 * @param button 按钮状态 (0=释放, 1=按下)
 * @param degree 旋钮角度 (-360~360)
 * @return bool 发送成功返回 true，失败返回 false
 */
bool Radial_SendData(__data uint8_t button, __data int16_t degree) {
    // 设置按钮状态（bit0）
    if (button & 0x01) {
        radialReport.buttonDial |= RADIAL_BUTTON_MASK;
    } else {
        radialReport.buttonDial &= ~RADIAL_BUTTON_MASK;
    }

    // 设置旋钮值（bit1-15）
    if (degree < -360) {
        degree = -360;
    } else if (degree > 360) {
        degree = 360;
    }

    // 首先清除旋钮位
    radialReport.buttonDial &= RADIAL_BUTTON_MASK;
    // 然后设置旋钮值（旋钮值从bit1开始）
    radialReport.buttonDial |= ((uint16_t)(degree * 10)) & RADIAL_DIAL_MASK;

    // 发送报告
    return Radial_SendReport(&radialReport);
}

/**
 * @brief 重置径向控制器报告
 */
void Radial_ResetReport(void) {
    radialReport.reportId = RADIAL_REPORT_ID;
    radialReport.buttonDial = 0;
}

/**
 * @brief 获取径向控制器报告指针
 * @return RadialReport* 径向控制器报告指针
 */
RadialReport *Radial_GetReport(void) { return &radialReport; }

/**
 * @brief 获取最后一个错误代码
 * @return uint8_t 错误代码
 */
uint8_t Radial_GetLastError(void) { return lastError; }
