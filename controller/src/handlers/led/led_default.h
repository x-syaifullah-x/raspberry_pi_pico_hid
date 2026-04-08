#pragma once

#include "hardware/gpio.h"
#include "hardware/sync.h"
#include "packet/rx/packet_rx.h"
#include "packet/tx/packet_tx.h"
#include "pico/time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) {
    uint8_t mode;
    uint16_t delay_ms;
} led_t;

#define DEFAULT_DELAY_MS 1000

#define LED_DEFAULT_DELAY_STEP_MS 100
#define LED_DEFAULT_TIMER_DELAY_US_TO_MS(t) ((t).delay_us / 1000)

#define LED_DEFAULT_OFF 0x00
#define LED_DEFAULT_ON 0x01
#define LED_DEFAULT_BLINK 0x02
#define LED_DEFAULT_STATE 0xFF

volatile bool led_blink_stat = false;

struct repeating_timer led_blink_timer;

static inline bool led_blink(struct repeating_timer* timer) {
    (void)timer;

    static bool state = false;
    gpio_put(PICO_DEFAULT_LED_PIN, (state = !state));
    return true;
}

static inline uint8_t led_get_mode() {
    return gpio_get(PICO_DEFAULT_LED_PIN);
}

static inline led_t led_default(bool on) {
    led_blink_stat = false;
    uint32_t irq_flags = save_and_disable_interrupts();
    cancel_repeating_timer(&led_blink_timer);
    restore_interrupts(irq_flags);
    gpio_put(PICO_DEFAULT_LED_PIN, on);
    return (led_t){.mode = led_get_mode(), .delay_ms = 0x00};
}

static inline led_t led_default_blink(uint32_t delay_ms) {
    uint32_t irq_flags = save_and_disable_interrupts();
    cancel_repeating_timer(&led_blink_timer);
    restore_interrupts(irq_flags);
    led_blink_stat = add_repeating_timer_ms(delay_ms, led_blink, NULL, &led_blink_timer);
    return (led_t){.mode = LED_DEFAULT_BLINK, .delay_ms = LED_DEFAULT_TIMER_DELAY_US_TO_MS(led_blink_timer)};
}

static inline led_t led_default_status() {
    led_t res;
    if (led_blink_stat) {
        res.mode = LED_DEFAULT_BLINK;
        res.delay_ms = LED_DEFAULT_TIMER_DELAY_US_TO_MS(led_blink_timer);
    } else {
        res.mode = led_get_mode();
    }
    return res;
}

void led_default_init(void) {
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    led_blink_stat = add_repeating_timer_ms(DEFAULT_DELAY_MS, led_blink, NULL, &led_blink_timer);
}

static inline packet_tx_t handle_led_default(packet_rx_t packet_rx) {
    packet_tx_t packet_tx = {.id = packet_rx.id, .status = STATUS_OK, .cmd = packet_rx.cmd};
    led_t led;
    switch (packet_rx.args[0]) {
        case LED_DEFAULT_OFF:
        case LED_DEFAULT_ON:
            led = led_default(packet_rx.args[0]);
            break;
        case LED_DEFAULT_BLINK:;
            uint8_t delay_step = packet_rx.args[1];
            if (!delay_step) {
                packet_tx.status = STATUS_ERR;
                packet_tx.data[0] = STATUS_ERR_CMD_ARG_INVALID;
                return packet_tx;
            }
            led = led_default_blink(delay_step * LED_DEFAULT_DELAY_STEP_MS);
            break;
        case LED_DEFAULT_STATE:
            led = led_default_status();
            break;
        default:
            packet_tx.status = STATUS_ERR;
            packet_tx.data[0] = STATUS_ERR_CMD_ARG_INVALID;
            return packet_tx;
    }

    packet_tx.data[0] = led.mode;
    packet_tx.data[1] = (led.delay_ms & 0xFF);
    packet_tx.data[2] = ((led.delay_ms >> 8) & 0xFF);
    return packet_tx;
}

#ifdef __cplusplus
}
#endif