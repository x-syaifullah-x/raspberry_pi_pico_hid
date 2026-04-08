#pragma once

#include <stdint.h>

#include "hal/pin.h"
#include "hardware/irq.h"
#include "hardware/regs/intctrl.h"
#include "hardware/uart.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_0_ENABLE 1

#define UART_0_ID uart0
#define UART_0_IRQ UART0_IRQ

void uart_0_init(pin_t rx_pin, pin_t tx_pin, uint32_t baudrate, irq_handler_t handler);

#ifdef __cplusplus
}
#endif