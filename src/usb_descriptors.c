#include <string.h>

#include "hid_report.h"
#include "tusb.h"

tusb_desc_device_t const desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,

    .bDeviceClass = 0,
    .bDeviceSubClass = 0,
    .bDeviceProtocol = 0,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = 0xCafe,
    .idProduct = 0x4000,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01};

uint8_t const *tud_descriptor_device_cb(void) {
    return (uint8_t const *)&desc_device;
}

#define CONFIG_TOTAL_LEN (TUD_CONFIG_DESC_LEN + TUD_HID_DESC_LEN)

uint8_t const desc_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, 1, 0, CONFIG_TOTAL_LEN, 0, 100),

    TUD_HID_DESCRIPTOR(0, 0, HID_ITF_PROTOCOL_NONE, HID_REPORT_DESC_LEN, 0x81, 64, 10)};

uint8_t const *tud_descriptor_configuration_cb(uint8_t index) {
    return desc_configuration;
}

const char *string_desc_arr[] = {
    (const char[]){0x09, 0x04},  // LANGID (English)
    "Raspberry PI Pico",         // Manufacturer
    "RP2040",                    // Product
    "xxx-1234",                  // SerialNumber
};

static uint16_t _desc_str[32];

uint16_t const *tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    uint8_t chr_count;
    if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0])) {
        return NULL;  // ❗ prevent crash
    }

    const char *str = string_desc_arr[index];
    chr_count = strlen(str);

    if (chr_count > 31) chr_count = 31;  // safety

    for (uint8_t i = 0; i < chr_count; i++) {
        _desc_str[1 + i] = str[i];
    }

    _desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return _desc_str;
}

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t *buffer, uint16_t reqlen) {
    return 0;
}