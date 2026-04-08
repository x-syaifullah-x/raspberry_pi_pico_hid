#pragma once

#include "handlers/adc.h"
#include "handlers/led/led.h"
#include "handlers/motor.h"
#include "handlers/system/bootrom.h"
#include "handlers/system/watcdog.h"
#include "packet/rx/packet_rx.h"
#include "packet/tx/packet_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline packet_tx_t dispatch(packet_rx_t rx) {
    switch (rx.cmd) {
        case CMD_SYSTEM_REBOOT:
            return handle_system_reboot(rx);
        case CMD_SYSTEM_RESET_USB:
            return handle_system_reset_usb(rx);
        // case CMD_SYSTEM_MEMORY:
        //     return handle_system_memory(rx);
        case CMD_LED_BEGIN ... CMD_LED_END:
            return handle_led(rx);
        case CMD_ADC_BEGIN ... CMD_ADC_END:
            return handle_adc(rx);
        case CMD_MOTOR:
            return handle_motor(rx);
        default:
            return (packet_tx_t){.id = rx.id, .status = STATUS_ERR, .cmd = rx.cmd, .data = {STATUS_ERR_CMD_UNKNOWN}};
    }
}

#ifdef __cplusplus
}
#endif