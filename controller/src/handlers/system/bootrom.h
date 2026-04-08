#pragma once

#include "packet/rx/packet_rx.h"
#include "packet/tx/packet_tx.h"
#include "pico/bootrom.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline packet_tx_t handle_system_reset_usb(packet_rx_t rx) {
    reset_usb_boot(0, rx.args[0]);
    return (packet_tx_t){.id = rx.id, .status = STATUS_OK, .cmd = rx.cmd};
}

#ifdef __cplusplus
}
#endif