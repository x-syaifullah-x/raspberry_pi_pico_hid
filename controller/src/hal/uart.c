#include "hal/uart.h"

#include "hardware/gpio.h"

void uart_0_init(pin_t rx_pin, pin_t tx_pin, uint32_t baudrate, irq_handler_t handler) {
#if UART_0_ENABLE

    if (uart_is_enabled(UART_0_ID))
        return;

    uart_init(UART_0_ID, baudrate);

    gpio_set_function(rx_pin, GPIO_FUNC_UART);
    gpio_set_function(tx_pin, GPIO_FUNC_UART);

    uart_set_fifo_enabled(UART_0_ID, true);

    irq_set_exclusive_handler(UART_0_IRQ, handler);
    irq_set_enabled(UART_0_IRQ, true);

    uart_set_irq_enables(UART_0_ID, true, false);

#else
    (void)rx_pin;
    (void)tx_pin;
    (void)baudrate;
    (void)handler;
#endif
}