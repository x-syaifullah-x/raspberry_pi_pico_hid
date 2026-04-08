#include <string.h>
#include <sys/param.h>

#include "driver/gpio.h"
#include "esp_log.h"
#include "led/led_default.h"
#include "nvs_flash.h"
#include "server/server.h"
#include "uart/uart1.h"
#include "wifi/wifi.h"
#if IP_NAPT
#include "lwip/lwip_napt.h"
#endif

static inline int ws_process(const uint8_t* payload, size_t payload_len) {
    return uart_write_bytes(UART_1_PORT, payload, payload_len);
}

static void stream_task(void* arg) {
    httpd_handle_t server = (httpd_handle_t)arg;
    uart_event_t event;
    uint8_t rx[UART_1_BUF_SIZE];

    while (true) {
        if (xQueueReceive(s_uart_1_queue, &event, portMAX_DELAY)) {
            switch (event.type) {
                case UART_DATA: {
                    uint32_t size = MIN(event.size, sizeof(rx));
                    int len = uart_read_bytes(UART_1_PORT, rx, size, 0);
                    if ((len % 8) != 0) break;
                    const uint8_t size_rx = 8;
                    const uint8_t size_tx = 7;
                    for (uint32_t i = 0; i + size_tx < len; i += size_rx) {
                        const uint8_t* tx = &rx[i + 1];
                        ESP_LOG_BUFFER_HEXDUMP("TX", tx, size_tx, ESP_LOG_INFO);
                        const pico_status_t status = tx[0];
                        switch (status) {
                            case PICO_STATUS_OK:
                                const pico_cmd_t cmd = tx[1];
                                ws_state_t* s = pico_state_map[cmd];
                                if (s)
                                    memcpy(s->data, tx + 2, sizeof(pico_data_t));
                                ws_broadcast(server, tx, size_tx);
                                break;
                            case PICO_STATUS_ERR:
                                ws_send(server, rx[i], tx, size_tx);
                                break;
                        }
                    }
                    break;
                }
                case UART_FIFO_OVF:
                    ESP_LOGW(TAG_WS, "UART FIFO overflow");
                    uart_flush_input(UART_1_PORT);
                    xQueueReset(s_uart_1_queue);
                    break;
                case UART_BUFFER_FULL:
                    ESP_LOGW(TAG_WS, "UART buffer full");
                    uart_flush_input(UART_1_PORT);
                    xQueueReset(s_uart_1_queue);
                    break;
                default:
                    break;
            }
        }
    }
}

void app_main(void) {
    // esp_log_level_set("*", ESP_LOG_NONE);

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    uart_init();

    xTaskCreate(stream_task, "stream_task", 2048, server_start(80), 5, NULL);

    uart_write_bytes(UART_1_PORT, (uint8_t[]){0, PICO_CMD_LED_DEFAULT, 255}, 8);
    uart_write_bytes(UART_1_PORT, (uint8_t[]){0, PICO_CMD_ADC_DMA, 255}, 8);
    uart_write_bytes(UART_1_PORT, (uint8_t[]){0, PICO_CMD_ADC_DMA, 255}, 8);
    uart_write_bytes(UART_1_PORT, (uint8_t[]){0, PICO_CMD_MOTOR, 255, PICO_CMD_MOTOR_1 - PICO_CMD_MOTOR}, 8);
    uart_write_bytes(UART_1_PORT, (uint8_t[]){0, PICO_CMD_MOTOR, 255, PICO_CMD_MOTOR_2 - PICO_CMD_MOTOR}, 8);
    uart_write_bytes(UART_1_PORT, (uint8_t[]){0, PICO_CMD_MOTOR, 255, PICO_CMD_MOTOR_3 - PICO_CMD_MOTOR}, 8);

    wifi_init();

    gpio_set_direction(GPIO_NUM_8, GPIO_MODE_OUTPUT);
    const esp_timer_create_args_t timer_args = {.callback = led_default_blink_cb, .name = "led_default_blink"};
    esp_timer_handle_t timer;
    esp_timer_create(&timer_args, &timer);
    esp_timer_start_periodic(timer, 1000000);
}