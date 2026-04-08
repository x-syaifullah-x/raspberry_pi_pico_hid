#pragma once

#include <stdint.h>
#include <sys/cdefs.h>

#include "packet/packet.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    uint32_t raw[2];
    struct {
        id_t id;
        status_t status;
        cmd_t cmd;
        uint8_t data[5];
    };
} packet_tx_t;

_Static_assert((sizeof(packet_tx_t) & (sizeof(packet_tx_t) - 1)) == 0, "packet_tx_t size must be power of two");

#ifdef __cplusplus
}
#endif