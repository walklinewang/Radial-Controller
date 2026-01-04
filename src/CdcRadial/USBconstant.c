/*
 created by Deqing Sun for use with CH55xduino
 */

#include "USBconstant.h"

// Device descriptor
__code USB_Descriptor_Device_t DeviceDescriptor = {
    .Header = {.Size = sizeof(USB_Descriptor_Device_t), .Type = DTYPE_Device},

    .USBSpecification = VERSION_BCD(1, 1, 0),
    .Class = 0xEF, // Miscellaneous
    .SubClass = 0x02,
    .Protocol = 0x01, // Interface Association Descriptor

    .Endpoint0Size = DEFAULT_ENDP0_SIZE,

    .VendorID = 0x1209,
    .ProductID = 0xc55c,
    .ReleaseNumber = VERSION_BCD(1, 0, 1),

    .ManufacturerStrIndex = 1,
    .ProductStrIndex = 2,
    .SerialNumStrIndex = 3,

    .NumberOfConfigurations = 1
};

/** Configuration descriptor structure. This descriptor, located in FLASH
 * memory, describes the usage of the device in one of its supported
 * configurations, including information about any device interfaces and
 * endpoints. The descriptor is read out by the USB host during the enumeration
 * process when selecting a configuration so that the host may correctly
 * communicate with the USB device.
 */
__code USB_Descriptor_Configuration_t ConfigurationDescriptor = {
    .Config = {.Header = {.Size = sizeof(USB_Descriptor_Configuration_Header_t),
                          .Type = DTYPE_Configuration},

               .TotalConfigurationSize = sizeof(USB_Descriptor_Configuration_t),
               .TotalInterfaces = 3,

               .ConfigurationNumber = 1,
               .ConfigurationStrIndex = NO_DESCRIPTOR,

               .ConfigAttributes = (USB_CONFIG_ATTR_RESERVED),

               .MaxPowerConsumption = USB_CONFIG_POWER_MA(200)},

    .CDC_IAD = {.Header = {.Size =
                               sizeof(USB_Descriptor_Interface_Association_t),
                           .Type = DTYPE_InterfaceAssociation},

                .FirstInterfaceIndex = INTERFACE_ID_CDC_CCI,
                .TotalInterfaces = 2,

                .Class = CDC_CSCP_CDCClass,
                .SubClass = CDC_CSCP_ACMSubclass,
                .Protocol = CDC_CSCP_ATCommandProtocol,

                .IADStrIndex = 4},

    .CDC_CCI_Interface = {.Header = {.Size = sizeof(USB_Descriptor_Interface_t),
                                     .Type = DTYPE_Interface},

                          .InterfaceNumber = INTERFACE_ID_CDC_CCI,
                          .AlternateSetting = 0,

                          .TotalEndpoints = 1,

                          .Class = CDC_CSCP_CDCClass,
                          .SubClass = CDC_CSCP_ACMSubclass,
                          .Protocol = CDC_CSCP_ATCommandProtocol,

                          .InterfaceStrIndex = 4},
    // refer to usbcdc11.pdf
    .CDC_Functional_Header =
        {
            .Header = {.Size = sizeof(USB_CDC_Descriptor_FunctionalHeader_t),
                       .Type = CDC_DTYPE_CSInterface},
            .Subtype = CDC_DSUBTYPE_CSInterface_Header,

            .CDCSpecification = VERSION_BCD(1, 1, 0),
        },
    // Todo: check CDC_DSUBTYPE_CSInterface_CallManagement difference?
    .CDC_Functional_ACM =
        {
            .Header = {.Size = sizeof(USB_CDC_Descriptor_FunctionalACM_t),
                       .Type = CDC_DTYPE_CSInterface},
            .Subtype = CDC_DSUBTYPE_CSInterface_ACM,

            .Capabilities = 0x02, // No Send_Break, Yes  Set_Line_Coding,
                                  // Set_Control_Line_State, Get_Line_Coding,
                                  // and the notification Serial_State.
        },

    .CDC_Functional_Union =
        {
            .Header = {.Size = sizeof(USB_CDC_Descriptor_FunctionalUnion_t),
                       .Type = CDC_DTYPE_CSInterface},
            .Subtype = CDC_DSUBTYPE_CSInterface_Union,

            .MasterInterfaceNumber = INTERFACE_ID_CDC_CCI,
            .SlaveInterfaceNumber = INTERFACE_ID_CDC_DCI,
        },

    .CDC_NotificationEndpoint =
        {.Header = {.Size = sizeof(USB_Descriptor_Endpoint_t),
                    .Type = DTYPE_Endpoint},

         .EndpointAddress = CDC_NOTIFICATION_EPADDR,
         .Attributes =
             (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC | ENDPOINT_USAGE_DATA),
         .EndpointSize = CDC_NOTIFICATION_EPSIZE,
         .PollingIntervalMS = 0x40},

    .CDC_DCI_Interface = {.Header = {.Size = sizeof(USB_Descriptor_Interface_t),
                                     .Type = DTYPE_Interface},

                          .InterfaceNumber = INTERFACE_ID_CDC_DCI,
                          .AlternateSetting = 0,

                          .TotalEndpoints = 2,

                          .Class = CDC_CSCP_CDCDataClass,
                          .SubClass = CDC_CSCP_NoDataSubclass,
                          .Protocol = CDC_CSCP_NoDataProtocol,

                          .InterfaceStrIndex = 4},

    .CDC_DataOutEndpoint = {.Header = {.Size =
                                           sizeof(USB_Descriptor_Endpoint_t),
                                       .Type = DTYPE_Endpoint},

                            .EndpointAddress = CDC_RX_EPADDR,
                            .Attributes =
                                (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC |
                                 ENDPOINT_USAGE_DATA),
                            .EndpointSize = CDC_TXRX_EPSIZE,
                            .PollingIntervalMS = 0x00},

    .CDC_DataInEndpoint = {.Header = {.Size = sizeof(USB_Descriptor_Endpoint_t),
                                      .Type = DTYPE_Endpoint},

                           .EndpointAddress = CDC_TX_EPADDR,
                           .Attributes = (EP_TYPE_BULK | ENDPOINT_ATTR_NO_SYNC |
                                          ENDPOINT_USAGE_DATA),
                           .EndpointSize = CDC_TXRX_EPSIZE,
                           .PollingIntervalMS = 0x00},

    .HID_Interface = {.Header = {.Size = sizeof(USB_Descriptor_Interface_t),
                                 .Type = DTYPE_Interface},

                      .InterfaceNumber = INTERFACE_ID_HID,
                      .AlternateSetting = 0x00,

                      .TotalEndpoints = 1,

                      .Class = HID_CSCP_HIDClass,
                      .SubClass = HID_CSCP_BootSubclass,
                      .Protocol = HID_CSCP_KeyboardBootProtocol,

                      .InterfaceStrIndex = NO_DESCRIPTOR},

    .HID_KeyboardHID = {.Header = {.Size = sizeof(USB_HID_Descriptor_HID_t),
                                   .Type = HID_DTYPE_HID},

                        .HIDSpec = VERSION_BCD(1, 1, 0),
                        .CountryCode = 0x00,
                        .TotalReportDescriptors = 1,
                        .HIDReportType = HID_DTYPE_Report,
                        .HIDReportLength = sizeof(ReportDescriptor)},

    .HID_ReportINEndpoint = {.Header = {.Size =
                                            sizeof(USB_Descriptor_Endpoint_t),
                                        .Type = DTYPE_Endpoint},

                             .EndpointAddress = KEYBOARD_EPADDR,
                             .Attributes =
                                 (EP_TYPE_INTERRUPT | ENDPOINT_ATTR_NO_SYNC |
                                  ENDPOINT_USAGE_DATA),
                             .EndpointSize = KEYBOARD_EPSIZE,
                             .PollingIntervalMS = 10}
};

// String Descriptors
__code uint8_t LanguageDescriptor[] = {
    0x04, 0x03, 0x09, 0x04 // Language Descriptor
};

__code uint16_t SerialDescriptor[] = {
    // Serial String Descriptor
    (((10 + 1) * 2) | (DTYPE_String << 8)),
    'C', 'H', '5', '5', 'x', ' ',
    'D', 'I', 'A', 'L',
};

__code uint16_t ProductDescriptor[] = {
    // Produce String Descriptor
    (((17 + 1) * 2) | (DTYPE_String << 8)),
    'R', 'a', 'd', 'i', 'a', 'l', ' ',
    'C', 'o', 'n', 't', 'r', 'o', 'l', 'l', 'e', 'r',
};

__code uint16_t ManufacturerDescriptor[] = {
    // SDCC is little endian
    (((8 + 1) * 2) | (DTYPE_String << 8)),
    'W', 'a', 'l', 'k', 'l', 'i', 'n', 'e',
};

__code uint16_t CDCDescriptor[] = {
    (((10 + 1) * 2) | (DTYPE_String << 8)),
    'C', 'D', 'C', ' ',
    'S', 'e', 'r', 'i', 'a', 'l',
};

__code uint8_t ReportDescriptor[] = {
    // Integrated Radial Controller TLC
    0x05, 0x01,          // USAGE_PAGE (Generic Desktop)
    0x09, 0x0e,          // USAGE (System Multi-Axis Controller)
    0xa1, 0x01,          // COLLECTION (Application)
    0x85, 0x01,          //   REPORT_ID (Radial Controller)
    0x05, 0x0d,          //   USAGE_PAGE (Digitizers)
    0x09, 0x21,          //   USAGE (Puck)
    0xa1, 0x00,          //   COLLECTION (Physical)
    0x05, 0x09,          //     USAGE_PAGE (Buttons)
    0x09, 0x01,          //     USAGE (Button 1)
    0x95, 0x01,          //     REPORT_COUNT (1)
    0x75, 0x01,          //     REPORT_SIZE (1)
    0x15, 0x00,          //     LOGICAL_MINIMUM (0)
    0x25, 0x01,          //     LOGICAL_MAXIMUM (1)
    0x81, 0x02,          //     INPUT (Data,Var,Abs)
    0x05, 0x01,          //     USAGE_PAGE (Generic Desktop)
    0x09, 0x37,          //     USAGE (Dial)
    0x95, 0x01,          //     REPORT_COUNT (1)
    0x75, 0x0f,          //     REPORT_SIZE (15)
    0x55, 0x0f,          //     UNIT_EXPONENT (-1)
    0x65, 0x14,          //     UNIT (Degrees, English Rotation)
    0x36, 0xf0, 0xf1,    //     PHYSICAL_MINIMUM (-3600)
    0x46, 0x10, 0x0e,    //     PHYSICAL_MAXIMUM (3600)
    0x16, 0xf0, 0xf1,    //     LOGICAL_MINIMUM (-3600)
    0x26, 0x10, 0x0e,    //     LOGICAL_MAXIMUM (3600)
    0x81, 0x06,          //     INPUT (Data,Var,Rel)

    // 0x09, 0x30,          //     USAGE (X)
    // 0x75, 0x10,          //     REPORT_SIZE (16)
    // 0x55, 0x0d,          //     UNIT_EXPONENT (-3)
    // 0x65, 0x13,          //     UNIT (Inch,EngLinear)
    // 0x35, 0x00,          //     PHYSICAL_MINIMUM (0)
    // 0x46, 0xc0, 0x5d,    //     PHYSICAL_MAXIMUM (24000)
    // 0x15, 0x00,          //     LOGICAL_MINIMUM (0)
    // 0x26, 0xff, 0x7f,    //     LOGICAL_MAXIMUM (32767)
    // 0x81, 0x02,          //     INPUT (Data,Var,Abs) 
    // 0x09, 0x31,          //     USAGE (Y)
    // 0x46, 0xb0, 0x36,    //     PHYSICAL_MAXIMUM (14000)
    // 0x81, 0x02,          //     INPUT (Data,Var,Abs)
    // 0x05, 0x0d,          //     USAGE_PAGE (Digitizers)
    // 0x09, 0x48,          //     USAGE (Width)
    // 0x36, 0xb8, 0x0b,    //     PHYSICAL_MINIMUM (3000)
    // 0x46, 0xb8, 0x0b,    //     PHYSICAL_MAXIMUM (3000)
    // 0x16, 0xb8, 0x0b,    //     LOGICAL_MINIMUM (3000)
    // 0x26, 0xb8, 0x0b,    //     LOGICAL_MAXIMUM (3000)
    // 0x81, 0x03,          //     INPUT (Cnst,Var,Abs)

    0xc0,                //   END_COLLECTION (Physical)

    // 0x85, 0x02,          //   REPORT_ID (Haptic Feedback)
    // 0x05, 0x0e,          //   USAGE_PAGE (Haptics)
    // 0x09, 0x01,          //   USAGE (Simple Haptic Controller)
    // 0xa1, 0x02,          //   COLLECTION (Logical)
    // 0x09, 0x20,          //     USAGE (Auto Trigger)
    // 0x16, 0x00, 0x10,    //     LOGICAL_MINIMUM (0x1000)
    // 0x26, 0x04, 0x10,    //     LOGICAL_MAXIMUM (0x1004)
    // 0x95, 0x01,          //     REPORT_COUNT (1)
    // 0x75, 0x10,          //     REPORT_SIZE (16)
    // 0xb1, 0x02,          //     FEATURE (Data,Var,Abs)
    // 0x09, 0x21,          //     USAGE (Manual Trigger)
    // 0x91, 0x02,          //     OUTPUT (Data,Var,Abs)
    // 0x09, 0x22,          //     USAGE (Auto Trigger Associated Control)
    // 0x17, 0x37,
    // 0x00, 0x01, 0x00,    //     LOGICAL_MINIMUM (0x00010037)
    // 0x27, 0x37,
    // 0x00, 0x01, 0x00,    //     LOGICAL_MAXIMUM (0x00010037)
    // 0x95, 0x01,          //     REPORT_COUNT (1)
    // 0x75, 0x20,          //     REPORT_SIZE (32)
    // 0xb1, 0x03,          //     FEATURE (Cnst,Var,Abs)
    // 0x09, 0x23,          //     USAGE (Intensity)
    // 0x15, 0x00,          //     LOGICAL_MINIMUM (0)
    // 0x25, 0x7f,          //     LOGICAL_MAXIMUM (127)
    // 0x75, 0x08,          //     REPORT_SIZE (8)
    // 0x91, 0x02,          //     OUTPUT (Data,Var,Abs)
    // 0x09, 0x23,          //     USAGE (Intensity)
    // 0xb1, 0x02,          //     FEATURE (Data,Var,Abs)
    // 0x09, 0x24,          //     USAGE (Repeat Count)
    // 0x91, 0x02,          //     OUTPUT (Data,Var,Abs)
    // 0x09, 0x24,          //     USAGE (Repeat Count)
    // 0xb1, 0x02,          //     FEATURE (Data,Var,Abs)
    // 0x09, 0x25,          //     USAGE (Retrigger Period)
    // 0x91, 0x02,          //     OUTPUT (Data,Var,Abs)
    // 0x09, 0x25,          //     USAGE (Retrigger Period)
    // 0xb1, 0x02,          //     FEATURE (Data,Var,Abs)
    // 0x09, 0x28,          //     USAGE (Waveform Cutoff Time)
    // 0x26, 0xff, 0x7f,    //     LOGICAL_MAXIMUM (32,767)
    // 0x75, 0x10,          //     REPORT_SIZE (16)
    // 0xb1, 0x02,          //     FEATURE (Data,Var,Abs)
    // 0x05, 0x0e,          //     USAGE_PAGE (Haptics)
    // 0x09, 0x10,          //     USAGE (Waveform List)
    // 0xa1, 0x02,          //     COLLECTION (Logical)
    // 0x05, 0x0A,          //       USAGE_PAGE (Ordinal)
    // 0x09, 0x03,          //       USAGE (Ordinal 3)
    // 0x95, 0x01,          //       REPORT_COUNT (1)
    // 0x75, 0x08,          //       REPORT_SIZE (8)
    // 0x15, 0x03,          //       LOGICAL_MINIMUM (3)
    // 0x25, 0x03,          //       LOGICAL_MAXIMUM (3)
    // 0x36, 0x03, 0x10,    //       PHYSICAL_MINIMUM (0x1003)
    // 0x46, 0x03, 0x10,    //       PHYSICAL_MAXIMUM (0x1003)
    // 0xb1, 0x03,          //       FEATURE (Cnst,Var,Abs)
    // 0x09, 0x04,          //       USAGE (Ordinal 4)
    // 0x15, 0x04,          //       LOGICAL_MINIMUM (4)
    // 0x25, 0x04,          //       LOGICAL_MAXIMUM (4)
    // 0x36, 0x04, 0x10,    //       PHYSICAL_MINIMUM (0x1004)
    // 0x46, 0x04, 0x10,    //       PHYSICAL_MAXIMUM (0x1004)
    // 0xb1, 0x03,          //       FEATURE (Cnst,Var,Abs)
    // 0xc0,                //     END_COLLECTION (Logical)
    // 0x05, 0x0e,          //     USAGE_PAGE (Haptics)
    // 0x09, 0x11,          //     USAGE (Duration List)
    // 0xa1, 0x02,          //     COLLECTION (Logical)
    // 0x05, 0x0a,          //       USAGE_PAGE (Ordinal)
    // 0x09, 0x03,          //       USAGE (Ordinal 3)
    // 0x09, 0x04,          //       USAGE (Ordinal 4)
    // 0x15, 0x00,          //       LOGICAL_MINIMUM (0)
    // 0x26, 0xff, 0x0f,    //       LOGICAL_MAXIMUM (4095)
    // 0x95, 0x02,          //       REPORT_COUNT (2)
    // 0x75, 0x08,          //       REPORT_SIZE (8)
    // 0xb1, 0x02,          //       FEATURE (Data,Var,Abs)
    // 0xc0,                //     END_COLLECTION (Logical)
    // 0xc0,                //   END_COLLECTION (Logical)
    0xc0,                // END_COLLECTION (Application)
};
