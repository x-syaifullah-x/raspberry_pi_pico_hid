#pragma once

#include "hal/pin.h"
#include "handlers/ws2812/ws2812.pio.h"

#ifdef __cplusplus
extern "C" {
#endif

#define WS2812_PIN PIN_21
#define IS_RGBW false

static inline void put_pixel(PIO pio, uint sm, uint32_t pixel_grb) {
    pio_sm_put_blocking(pio, sm, pixel_grb << 8u);
}

static inline uint32_t urgb_u32(uint8_t r, uint8_t g, uint8_t b) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | (uint32_t)(b);
}

static inline uint32_t urgbw_u32(uint8_t r, uint8_t g, uint8_t b, uint8_t w) {
    return ((uint32_t)(r) << 8) | ((uint32_t)(g) << 16) | ((uint32_t)(w) << 24) | (uint32_t)(b);
}

PIO pio = NULL;
uint sm = (uint)-1;
uint offset;

static inline bool ws2812_is_initialized(void) {
    return pio != NULL && sm != (uint)-1;
}

static inline void ws2812_init(void) {
    if (ws2812_is_initialized()) return;

    pio_claim_free_sm_and_add_program_for_gpio_range(&ws2812_program, &pio, &sm, &offset, WS2812_PIN, 1, true);
    ws2812_program_init(pio, sm, offset, WS2812_PIN, 800000, IS_RGBW);
}

static inline void ws2812_deinit(void) {
    pio_sm_set_enabled(pio, sm, false);
    pio_remove_program_and_unclaim_sm(&ws2812_program, pio, sm, offset);
    gpio_set_function(WS2812_PIN, GPIO_FUNC_SIO);
    gpio_disable_pulls(WS2812_PIN);
    gpio_set_dir(WS2812_PIN, GPIO_IN);

    sm = (uint)-1;
    pio = NULL;
}

/* code */
static inline packet_tx_t handle_led_WS2812_1BIT(packet_rx_t rx) {
    packet_tx_t tx = {.id = rx.id, .status = STATUS_OK, .cmd = rx.cmd, .data = {0}};

    switch (rx.args[0]) {
        case 0:  // init
            if (IS_RGBW) {
                put_pixel(pio, sm, 0);
            } else {
                put_pixel(pio, sm, 0);
            }

            sleep_us(30);

            rx.args[1] ? ws2812_init() : ws2812_deinit();
            tx.data[0] = rx.args[1];
            break;
        case 1:  // set
            if (!ws2812_is_initialized()) {
                tx.status = STATUS_ERR;
                tx.data[0] = STATUS_ERR_CMD_ARG_INVALID;
                return tx;
            }

            tx.data[0] = true;
            tx.data[1] = rx.args[1];
            tx.data[2] = rx.args[2];
            tx.data[3] = rx.args[3];
            if (IS_RGBW) {
                tx.data[4] = rx.args[4];
                put_pixel(pio, sm, urgbw_u32(tx.data[1], tx.data[2], tx.data[3], tx.data[4]));
            } else {
                put_pixel(pio, sm, urgb_u32(tx.data[1], tx.data[2], tx.data[3]));
            }
            break;
        default:
            tx.status = STATUS_ERR;
            tx.data[0] = STATUS_ERR_CMD_NOT_IMPLEMENTED;
            break;
    }

    return tx;
}

#ifdef __cplusplus
}
#endif