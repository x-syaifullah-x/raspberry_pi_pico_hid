#pragma once

#include <stdbool.h>
#include <stdint.h>
#include <sys/cdefs.h>

#include "packet_rx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PACKET_RX_RB_Q_SIZE 16
_Static_assert((PACKET_RX_RB_Q_SIZE & (PACKET_RX_RB_Q_SIZE - 1)) == 0, "PACKET_RX_RB_Q_SIZE must be power of two");
#define PACKET_RX_RB_Q_MASK (PACKET_RX_RB_Q_SIZE - 1)

static packet_rx_t packet_rx_rb_buf[PACKET_RX_RB_Q_SIZE];
static volatile uint8_t packet_rx_rb_head = 0;  // read
static volatile uint8_t packet_rx_rb_tail = 0;  // write

static inline bool packet_rx_rb_push(const packet_rx_t* packet_rx) {
    uint32_t next = (packet_rx_rb_tail + 1) & PACKET_RX_RB_Q_MASK;
    if (next == packet_rx_rb_head) return false;
    packet_rx_rb_buf[packet_rx_rb_tail] = *packet_rx;
    __asm__ volatile("" ::: "memory");  // compiler barrier
    packet_rx_rb_tail = next;
    return true;
}

static inline bool packet_rx_rb_pop(packet_rx_t* packet_rx) {
    if (packet_rx_rb_head == packet_rx_rb_tail) return false;  // empty
    *packet_rx = packet_rx_rb_buf[packet_rx_rb_head];
    __asm__ volatile("" ::: "memory");  // compiler barrier
    packet_rx_rb_head = (packet_rx_rb_head + 1) & PACKET_RX_RB_Q_MASK;
    return true;
}

#ifdef __cplusplus
}
#endif