#pragma once

#ifdef __cplusplus
extern "C" {
#endif

#define CFG_TUD_ENABLED 1
#if CFG_TUD_ENABLED
#if defined(LIB_PICO_STDIO_USB)
#define CFG_TUD_CDC 1
#define CFG_TUD_CDC_RX_BUFSIZE 64
#define CFG_TUD_CDC_TX_BUFSIZE 64
#else
#define CFG_TUD_CDC 0
#endif
#define CFG_TUD_HID 0
#define CFG_TUD_VENDOR 1
#define CFG_TUD_VENDOR_RX_BUFSIZE 0
#endif

#define CFG_TUSB_MCU OPT_MCU_RP2040
#define CFG_TUSB_DEBUG 0                         // 0=OFF| 1=ERR | 2=ERR + WARN | 3=ERR + WARN + INFO
#define CFG_TUSB_RHPORT0_MODE (OPT_MODE_DEVICE)  // USB MODE(OPT_MODE_DEVICE | OPT_MODE_HOST | OPT_MODE_NONE)

#define PICO_NO_BI_STDIO_USB 1

#ifdef __cplusplus
}
#endif