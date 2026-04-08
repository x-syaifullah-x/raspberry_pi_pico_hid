#pragma once

#include "driver/uart.h"
#include "esp_err.h"
#include "freertos/queue.h"

#ifdef __cplusplus
extern "C" {
#endif

#define UART_1_PORT UART_NUM_1
#define UART_1_TX_PIN GPIO_NUM_0
#define UART_1_RX_PIN GPIO_NUM_1
#define UART_1_BAUD 115200
#define UART_1_BUF_SIZE 256

static QueueHandle_t s_uart_1_queue;

static void uart_init(void) {
    const uart_config_t uart_config = {
        .baud_rate = UART_1_BAUD,
        .data_bits = UART_DATA_8_BITS,
        .parity = UART_PARITY_DISABLE,
        .stop_bits = UART_STOP_BITS_1,
        .flow_ctrl = UART_HW_FLOWCTRL_DISABLE,
        .source_clk = UART_SCLK_DEFAULT,
    };

    ESP_ERROR_CHECK(uart_param_config(UART_1_PORT, &uart_config));
    ESP_ERROR_CHECK(uart_set_pin(UART_1_PORT, UART_1_TX_PIN, UART_1_RX_PIN, UART_PIN_NO_CHANGE, UART_PIN_NO_CHANGE));
    ESP_ERROR_CHECK(uart_driver_install(UART_1_PORT, UART_1_BUF_SIZE, UART_1_BUF_SIZE, 8, &s_uart_1_queue, 0));
}

#ifdef __cplusplus
}
#endif