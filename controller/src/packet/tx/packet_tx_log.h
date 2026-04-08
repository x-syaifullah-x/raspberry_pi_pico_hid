#pragma once

#include <stdio.h>
#include <sys/cdefs.h>

#include "packet_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void packet_tx_log(const packet_tx_t* tx) {
#if defined(LIB_PICO_STDIO_USB)
    _Static_assert(sizeof(packet_tx_t) == 8, "packet_tx_t size changed, update packet_tx_log!");

    printf(
        "TX: │ id: %03u │ status: %03u │ cmd: %03u │ data: [%03u, %03u, %03u, %03u, %03u] │\n",
        tx->id, tx->status, tx->cmd, tx->data[0], tx->data[1], tx->data[2], tx->data[3], tx->data[4]);
#else
    (void)tx;
#endif
}

#ifdef __cplusplus
}
#endif