#pragma once

#include "packet/rx/packet_rx_process.h"
#include "pico/binary_info.h"

#ifdef __cplusplus
extern "C" {
#endif

// clang-format off
bi_decl(bi_program_version_string("1.0"))
bi_decl(bi_program_url("https://github.com/x-syaifullah-x/raspberry_pi_pico_commander"))
bi_decl(bi_program_description(
    "Device Controller\n"
    "ID Vendor      : " TU_XSTRING(USB_ID_VENDOR)"\n"
    "ID Product     : " TU_XSTRING(USB_ID_PRODUCT)"\n"
    "Manufacturer   : " USB_MANUFACTIRER"\n"
    "Product        : " USB_PRODUCT"\n\n"
    "Power Pins:\n"
    "VBUS    (5V USB)      -> PIN_40\n"
    "VSYS    (2-5V input)  -> PIN_39\n"
    "3V3OUT  (3.3V output) -> PIN_36\n"
    "GND     (Ground)      -> PIN_03, PIN_08, PIN_13, PIN_18, PIN_23, PIN_28, PIN_33, PIN_38"
))
    
static const char *string_desc_arr[] = {
    (const char[]){0x09, 0x04}, // Language ID(English US)
    USB_MANUFACTIRER,
    USB_PRODUCT,
    USB_SERIAL_NUMBER,
    "CDC Control", // CDC Interface
    "CDC Data", // CDC Data Interface
    "HID Control", // HID Interface
    "Bulk Data", // Vendor Interface
};  // clang-format on

uint16_t const* tud_descriptor_string_cb(uint8_t index, uint16_t langid) {
    (void)langid;

    static uint16_t desc_str[32];
    uint8_t chr_count;

    if (index == 0) {
        memcpy(&desc_str[1], string_desc_arr[0], 2);
        chr_count = 1;
    } else {
        if (index >= sizeof(string_desc_arr) / sizeof(string_desc_arr[0]))
            return NULL;

        const char* str = string_desc_arr[index];
        chr_count = strlen(str);

        if (chr_count > 31) chr_count = 31;

        for (uint8_t i = 0; i < chr_count; i++) {
            desc_str[1 + i] = str[i];  // ASCII → UTF-16LE
        }
    }

    // Header
    desc_str[0] = (TUSB_DESC_STRING << 8) | (2 * chr_count + 2);
    return desc_str;
}

enum {
#if CFG_TUD_CDC
    ITF_NUM_CDC,
    ITF_NUM_CDC_DATA,
#endif
#if CFG_TUD_HID
    ITF_NUM_HID,
#endif
#if CFG_TUD_VENDOR
    ITF_NUM_VENDOR,
#endif
    ITF_NUM_TOTAL
};

#define USB_CDC_DESCRIPTOR_(_itfnum, _stridx, _stridx_data, _ep_notif, _ep_notif_size, _epout, _epin, _epsize)                                                                                                               \
    /* Interface Associate */                                                                                                                                                                                                \
    8, TUSB_DESC_INTERFACE_ASSOCIATION, _itfnum, 2, TUSB_CLASS_CDC, CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL, CDC_COMM_PROTOCOL_NONE, 0,                                                         /* CDC Control Interface */ \
        9, TUSB_DESC_INTERFACE, _itfnum, 0, 1, TUSB_CLASS_CDC, CDC_COMM_SUBCLASS_ABSTRACT_CONTROL_MODEL, CDC_COMM_PROTOCOL_NONE, _stridx,                                                        /* CDC Header */            \
        5, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_HEADER, U16_TO_U8S_LE(0x0120),                                                                                                                  /* CDC Call */              \
        5, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_CALL_MANAGEMENT, 0, (uint8_t)((_itfnum) + 1),                                                                                                   /* CDC ACM */               \
        4, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_ABSTRACT_CONTROL_MANAGEMENT, 6,                                                                                                                 /* CDC Union */             \
        5, TUSB_DESC_CS_INTERFACE, CDC_FUNC_DESC_UNION, _itfnum, (uint8_t)((_itfnum) + 1),                                                                                                       /* Endpoint Notification */ \
        7, TUSB_DESC_ENDPOINT, _ep_notif, TUSB_XFER_INTERRUPT, U16_TO_U8S_LE(_ep_notif_size), 1,                                                                                                 /* CDC Data Interface */    \
        9, TUSB_DESC_INTERFACE, (uint8_t)((_itfnum) + 1), 0, 2, TUSB_CLASS_CDC_DATA, 0, 0, _stridx_data, /*                                                                            ↑ fix! */ /* Endpoint Out */          \
        7, TUSB_DESC_ENDPOINT, _epout, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0,                                                                                                                /* Endpoint In */           \
        7, TUSB_DESC_ENDPOINT, _epin, TUSB_XFER_BULK, U16_TO_U8S_LE(_epsize), 0

#define USB_CDC_DESCRIPTOR USB_CDC_DESCRIPTOR_(ITF_NUM_CDC, 4, 5, 0x81, 8, 0x02, 0x82, 64)
#define USB_HID_DESCRIPTOR TUD_HID_INOUT_DESCRIPTOR(ITF_NUM_HID, 6, HID_ITF_PROTOCOL_NONE, sizeof(hid_descriptor_report), 0x83, 0x03, CFG_TUD_HID_EP_BUFSIZE, 1)
#define USB_VENDOR_DESCRIPTOR TUD_VENDOR_DESCRIPTOR(ITF_NUM_VENDOR, 7, 0x04, 0x84, CFG_TUD_VENDOR_EPSIZE)

// clang-format off
#if CFG_TUD_CDC && CFG_TUD_HID && CFG_TUD_VENDOR

bi_decl(bi_program_feature("USB vendor"))
bi_decl(bi_program_feature("USB hid"))
bi_decl(bi_program_feature("USB cdc"))

static uint8_t const descriptor_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_VENDOR_DESC_LEN), 0, 100),
    USB_CDC_DESCRIPTOR,
    USB_HID_DESCRIPTOR,
    USB_VENDOR_DESCRIPTOR,
};
#elif CFG_TUD_CDC && CFG_TUD_HID

bi_decl(bi_program_feature("USB hid"))
bi_decl(bi_program_feature("USB cdc"))

static uint8_t const descriptor_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_HID_INOUT_DESC_LEN), 0, 100),
    USB_CDC_DESCRIPTOR,
    USB_HID_DESCRIPTOR,
};
#elif CFG_TUD_CDC && CFG_TUD_VENDOR

bi_decl(bi_program_feature("USB vendor"))
bi_decl(bi_program_feature("USB cdc"))

static uint8_t const descriptor_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN + TUD_VENDOR_DESC_LEN), 0, 100),
    USB_CDC_DESCRIPTOR,
    USB_VENDOR_DESCRIPTOR,
};
#elif CFG_TUD_HID && CFG_TUD_VENDOR

bi_decl(bi_program_feature("USB vendor"))
bi_decl(bi_program_feature("USB hid"))

static uint8_t const descriptor_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN + TUD_VENDOR_DESC_LEN), 0, 100),
    USB_HID_DESCRIPTOR,
    USB_VENDOR_DESCRIPTOR,
};
#elif CFG_TUD_CDC

bi_decl(bi_program_feature("USB cdc"))

static uint8_t const descriptor_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, (TUD_CONFIG_DESC_LEN + TUD_CDC_DESC_LEN), 0, 100),
    USB_CDC_DESCRIPTOR,
};
#elif CFG_TUD_HID

bi_decl(bi_program_feature("USB hid"))

static uint8_t const descriptor_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, (TUD_CONFIG_DESC_LEN + TUD_HID_INOUT_DESC_LEN), 0, 100),
    USB_HID_DESCRIPTOR,
};
#elif CFG_TUD_VENDOR

bi_decl(bi_program_feature("USB vendor"))

static uint8_t const descriptor_configuration[] = {
    TUD_CONFIG_DESCRIPTOR(1, ITF_NUM_TOTAL, 0, (TUD_CONFIG_DESC_LEN + TUD_VENDOR_DESC_LEN), 0, 100),
    USB_VENDOR_DESCRIPTOR,
};
#else
#undef CFG_TUD_ENABLED
#endif
// clang-format on

#if CFG_TUD_ENABLED
uint8_t const* tud_descriptor_configuration_cb(uint8_t index) {
    (void)index;

    return descriptor_configuration;
}

const tusb_desc_device_t desc_device = {
    .bLength = sizeof(tusb_desc_device_t),
    .bDescriptorType = TUSB_DESC_DEVICE,
    .bcdUSB = 0x0200,

    .bDeviceClass = 0x00,
    .bDeviceSubClass = 0x00,
    .bDeviceProtocol = 0x00,

    .bMaxPacketSize0 = CFG_TUD_ENDPOINT0_SIZE,

    .idVendor = USB_ID_VENDOR,
    .idProduct = USB_ID_PRODUCT,
    .bcdDevice = 0x0100,

    .iManufacturer = 0x01,
    .iProduct = 0x02,
    .iSerialNumber = 0x03,

    .bNumConfigurations = 0x01,
};

uint8_t const* tud_descriptor_device_cb(void) {
    return (uint8_t const*)&desc_device;
}

#endif

#if CFG_TUD_CDC
void tud_cdc_rx_cb(uint8_t itf) {
    RX_PROCESS_FIFO(itf, tud_cdc_n_available, tud_cdc_n_read);
}
#endif

#if CFG_TUD_HID

#define INPUT_HID_REPORT_COUNT CFG_TUD_HID_EP_BUFSIZE
#define OUTPUT_HID_REPORT_COUNT CFG_TUD_HID_EP_BUFSIZE

static const uint8_t hid_descriptor_report[] = {
    HID_USAGE_PAGE_N(HID_USAGE_PAGE_VENDOR, 2),
    HID_USAGE(0x01),
    HID_COLLECTION(HID_COLLECTION_APPLICATION),
    /* Input */
    HID_USAGE(0x02),
    HID_LOGICAL_MIN(0x00),
    HID_LOGICAL_MAX_N(0xFF, 2),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(INPUT_HID_REPORT_COUNT),
    HID_INPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    /* Output */
    HID_USAGE(0x03),
    HID_LOGICAL_MIN(0x00),
    HID_LOGICAL_MAX_N(0xFF, 2),
    HID_REPORT_SIZE(8),
    HID_REPORT_COUNT(OUTPUT_HID_REPORT_COUNT),
    HID_OUTPUT(HID_DATA | HID_VARIABLE | HID_ABSOLUTE),
    HID_COLLECTION_END,
};

uint16_t tud_hid_get_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t* buffer, uint16_t reqlen) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    (void)buffer;
    (void)reqlen;

    return 0;
}

uint8_t const* tud_hid_descriptor_report_cb(uint8_t instance) {
    (void)instance;
    return hid_descriptor_report;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const* buffer, uint16_t bufsize) {
    (void)instance;
    (void)report_id;
    (void)report_type;
    RX_PROCESS_BUF(instance, buffer, bufsize, CFG_TUD_HID_EP_BUFSIZE);
}
#endif

#if CFG_TUD_VENDOR
void tud_vendor_rx_cb(uint8_t itf, uint8_t const* buffer, uint16_t bufsize) {
    RX_PROCESS_BUF(itf, buffer, bufsize);
}
#endif

#ifdef __cplusplus
}
#endif