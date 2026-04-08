#pragma once

#include "hardware/watchdog.h"
#include "packet/rx/packet_rx.h"
#include "packet/tx/packet_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline packet_tx_t handle_system_reboot(packet_rx_t rx) {
    watchdog_reboot(0, 0, 0);
    return (packet_tx_t){.id = rx.id, .status = STATUS_OK, .cmd = rx.cmd};
}

#ifdef __cplusplus
}
#endif