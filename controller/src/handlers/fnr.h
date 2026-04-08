#pragma once

#include "hardware/gpio.h"
#include "pin.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum {
    FNR_NEUTRAL,
    FNR_FORWARD,
    FNR_REVERSE,
    FNR_ERROR
} fnr_state_t;

static inline void fnr_init(pin_t forward, pin_t reverse) {
    gpio_init(forward);
    gpio_set_dir(forward, GPIO_IN);
    gpio_pull_up(forward);

    gpio_init(reverse);
    gpio_set_dir(reverse, GPIO_IN);
    gpio_pull_up(reverse);
}

static inline fnr_state_t fnr_read(void) {
    return (fnr_state_t)(~sio_hw->gpio_in) & 0x3u;
}

#ifdef __cplusplus
}
#endif