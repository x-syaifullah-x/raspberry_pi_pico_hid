#pragma once

#include "handlers/led/led_default.h"
#include "handlers/ws2812/ws2812.h"
#include "packet/rx/packet_rx.h"
#include "packet/tx/packet_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline packet_tx_t handle_led(packet_rx_t rx) {
    switch (rx.cmd) {
        case CMD_LED_DEFAULT:
            return handle_led_default(rx);
        case CMD_LED_WS2812_1BIT:
            return handle_led_WS2812_1BIT(rx);
        default:
            return (packet_tx_t){.id = rx.id, .cmd = rx.cmd, .status = STATUS_ERR, .data = {STATUS_ERR_CMD_UNKNOWN}};
    }
}

#ifdef __cplusplus
}
#endif