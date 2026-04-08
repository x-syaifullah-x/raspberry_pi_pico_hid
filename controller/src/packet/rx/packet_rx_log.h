#pragma once

#include <stdio.h>
#include <sys/cdefs.h>

#include "packet_rx.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void packet_rx_log(const packet_rx_t* rx) {
#if defined(LIB_PICO_STDIO_USB)
    _Static_assert(sizeof(packet_rx_t) == 8, "packet_rx_t size changed, update packet_rx_log!");

    printf(
        "RX: │ id: %03u │ cmd: %03u │ args: [%03u, %03u, %03u, %03u, %03u, %03u] │\n",
        rx->id, rx->cmd, rx->args[0], rx->args[1], rx->args[2], rx->args[3], rx->args[4], rx->args[5]);
#else
    (void)rx;
#endif
}

#ifdef __cplusplus
}
#endif