#pragma once

#include <stdint.h>

#include "hal/uart.h"
#include "hardware/uart.h"
#include "packet/rx/packet_rx.h"
#include "packet/rx/packet_rx_rb.h"

#ifdef __cplusplus
extern "C" {
#endif

static inline void uart_0_irq_handler(void) {
    static packet_rx_t rx;
    static uint32_t rx_len = 0;

    uint32_t rsr = uart_get_hw(UART_0_ID)->rsr;
    if (__builtin_expect(rsr, 0)) {
        uart_get_hw(UART_0_ID)->rsr = rsr;
        rx_len = 0;
        return;
    }

    uint8_t* rx_ptr = (uint8_t*)&rx;

    while (uart_is_readable(UART_0_ID)) {
        rx_ptr[rx_len++] = uart_getc(UART_0_ID);
        if (__builtin_expect(rx_len == sizeof(packet_rx_t), 0)) {
            rx_len = 0;
            packet_rx_rb_push(&rx);
        }
    }
}

#ifdef __cplusplus
}
#endif