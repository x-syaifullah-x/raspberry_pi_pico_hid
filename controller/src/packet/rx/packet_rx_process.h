#pragma once

#include <stdint.h>

#include "packet_rx.h"
#include "packet_rx_rb.h"
#include "pico/platform/compiler.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PACKET_RX_EP_CAPACITY (64 / sizeof(packet_rx_t))

#define RX_PROCESS_FIFO(INTERFACE, AVAILABLE, READ)                       \
    do {                                                                  \
        uint32_t count = TU_DIV_CEIL(AVAILABLE(INTERFACE), 8);            \
        if (!count) return;                                               \
        if (count > PACKET_RX_EP_CAPACITY) count = PACKET_RX_EP_CAPACITY; \
        packet_rx_t rx;                                                   \
        do {                                                              \
            const uint32_t read = READ(INTERFACE, &rx, sizeof(rx));       \
            if (read < (sizeof(rx.id) + sizeof(rx.cmd))) continue;        \
            packet_rx_rb_push(&rx);                                       \
        } while (--count);                                                \
    } while (0)

#define RX_PROCESS_BUF(INTERFACE, BUFFER, BUFSIZE)                              \
    do {                                                                        \
        (void)INTERFACE;                                                        \
                                                                                \
        const packet_rx_t* end = (const packet_rx_t*)(BUFFER + BUFSIZE);        \
        for (const packet_rx_t* p = (const packet_rx_t*)BUFFER; p < end; p++) { \
            packet_rx_rb_push(p);                                               \
        }                                                                       \
    } while (0)

#ifdef __cplusplus
}
#endif