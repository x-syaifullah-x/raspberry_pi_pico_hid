#include <stdint.h>

#include "pi_pico_data.h"

#ifndef _HID_REPORT_
#define _HID_REPORT_

static const uint8_t hid_report_desc[] = {
    0x06, 0x00, 0xFF,  // Usage Page (Vendor Defined - custom device)
    0x09, 0x01,        // Usage (custom usage ID 1)

    0xA1, 0x01,  // Collection (Application - start of HID device)

    0x15, 0x00,        // Logical Minimum (0)
    0x26, 0xFF, 0x00,  // Logical Maximum (255)

    0x75, 0x08,                  // Report Size (8 bits per field = 1 byte)
    0x95, sizeof(pi_pico_data),  // Report Count (1-byte fields, matches packed struct size)
    // 0x09, 0x01,  // Usage (custom usage ID 1)
    0x81, 0x02,  // Input (Data, Variable, Absolute) - device sends data to host

    // 0x75, 0x08,  // Report Size (8 bits per field = 1 byte)
    // 0x95, 0x01,  // Report Count (8 bytes again)
    // 0x09, 0x01,  // Usage (custom usage ID 1)
    // 0x91, 0x02,  // Output (Data, Variable, Absolute) - host sends data to device

    0xC0  // End Collection
};

#define HID_REPORT_DESC_LEN (sizeof(hid_report_desc))

#endif