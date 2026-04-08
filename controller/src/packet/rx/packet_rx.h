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
        cmd_t cmd;
        uint8_t args[6];
    };
} packet_rx_t;

_Static_assert((sizeof(packet_rx_t) & (sizeof(packet_rx_t) - 1)) == 0, "packet_rx_t size must be power of two");

#ifdef __cplusplus
}
#endif