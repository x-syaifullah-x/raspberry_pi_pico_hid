#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/watchdog.h"
#include "methode.h"
#include "pi_pico_data.h"
#include "pico/bootrom.h"
#include "pin.h"
#include "tb_6612_fng.h"
#include "tusb.h"

volatile bool led_blink_stat = false;

struct repeating_timer timer;

static inline bool led_blink(struct repeating_timer *t) {
    static bool state = false;
    gpio_put(PICO_DEFAULT_LED_PIN, (state = !state));
    return true;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *params, uint16_t params_size) {
    pi_pico_data data = {.err = 0x00, .methode = report_id, .gpio = 0xFF, .value = 0x00};

    switch (data.methode) {
        // printf '\x00\[xPIN_N]' > /dev/hidrawN
        case METHODE_GET_GPIO:
            data.gpio = params[0];
            if (data.gpio > 28) {
                data.err = INVALID_METHOD_PARAM;
                break;
            }
            data.value = (data.gpio == PICO_DEFAULT_LED_PIN && led_blink_stat) ? 0x02 : gpio_get(data.gpio);
            break;
        // printf '\x01\x[00(OFF) | 01(ON) | 02(BLINK)]' > /dev/hidrawN
        case METHODE_SET_DEFAULT_LED:
            data.gpio = PICO_DEFAULT_LED_PIN;
            uint32_t flags;
            switch (params[0]) {
                case 0x00:
                case 0x01:
                    led_blink_stat = false;
                    flags = save_and_disable_interrupts();
                    cancel_repeating_timer(&timer);
                    restore_interrupts(flags);
                    gpio_put(data.gpio, params[0]);
                    data.value = gpio_get(data.gpio);
                    break;
                case 0x02:
                    flags = save_and_disable_interrupts();
                    cancel_repeating_timer(&timer);
                    restore_interrupts(flags);
                    led_blink_stat = add_repeating_timer_ms(((params_size >= 2) ? (params[1] * 100) : 1000), led_blink, NULL, &timer);
                    data.value = 0x02;
                    break;
                default:
                    data.err = INVALID_METHOD_PARAM;
                    break;
            }
            break;
        // printf '\x02\x[04(ONBOARD_TEMP)]' > /dev/hidrawN
        case METHODE_ADC_READ:
            adc_select_input(params[0]);
            data.value = adc_read() /* 0 – 4095 */;
            // float voltage = (data.value * 3.3F) / 4095F;
            // float temp = 27 - (voltage - 0.706f) / 0.001721f;
            break;
        case METHODE_TB6612FNG:
            switch (params[0]) {
                // printf '\x03\x00\x[00(OFF) || 01(ON)]' > /dev/hidrawN
                case METHODE_TB6612FNG_POWER:
                    switch (params[1]) {
                        case METHODE_TB6612FNG_POWER_OFF:
                            tb_6612_fng_off(PIN_05, PIN_01, PIN_02, PIN_04);
                            data.value = 0x00;
                            break;
                        case METHODE_TB6612FNG_POWER_ON:
                            if (!tb_6612_fng_on(PIN_05, PIN_01, PIN_02, PIN_04)) {
                                data.value = 0x01;
                                break;
                            }
                            data.value = 0x01;
                            break;
                        default:
                            data.err = INVALID_METHOD_PARAM;
                            break;
                    }
                    break;
                // printf '\x03\x01\x[01(MOTOR_1) || 02(MOTOR_2)]\x[00(FREE) || 01(FORWARD) || 02(REVERSE) || 03(BREAK)]\x[0 - 100 (SPEED)]' > /dev/hidrawN
                case METHODE_TB6612FNG_MOTOR:
                    switch (params[1]) {
                        case METHODE_TB6612FNG_MOTOR_1:
                            switch (params[2]) {
                                case 0x00:  // FREE
                                    if (!tb_6612_fng_free(PIN_05, PIN_01, PIN_02))
                                        data.err = INVALID_METHOD_PARAM;
                                    break;
                                case 0x01:  // FORWARD
                                    if (!tb_6612_fng_forward(PIN_05, PIN_01, PIN_02, params[3]))
                                        data.err = INVALID_METHOD_PARAM;
                                    break;
                                case 0x02:  // REVERSE
                                    if (!tb_6612_fng_reverse(PIN_05, PIN_01, PIN_02, params[3]))
                                        data.err = INVALID_METHOD_PARAM;
                                    break;
                                case 0x03:  // BREAK
                                    if (!tb_6612_fng_break(PIN_05, PIN_01, PIN_02))
                                        data.err = INVALID_METHOD_PARAM;
                                    break;
                                default:
                                    data.err = INVALID_METHOD_PARAM;
                                    break;
                            }
                            break;
                        case METHODE_TB6612FNG_MOTOR_2:
                            data.err = INVALID_METHOD_PARAM;
                            break;
                        default:
                            data.err = INVALID_METHOD_PARAM;
                            break;
                    }
                    break;
                default:
                    data.err = INVALID_METHOD_PARAM;
                    break;
            }
            break;
        // printf '\xFF\x[00(REBOOT) | 01(BOOTSEL) | 02(READ_TEMP)]' > /dev/hidrawN
        case METHODE_SYSTEM:
            switch (params[0]) {
                case METHODE_SYSTEM_REBOOT:
                    watchdog_reboot(0, 0, 0);
                    return;
                case METHODE_SYSTEM_REBOOT_BOOTSEL:
                    reset_usb_boot(0, 0);
                    return;
                default:
                    data.err = INVALID_METHOD_PARAM;
                    break;
            }
            break;
        default:
            data.err = INVALID_METHODE_NUMBER;
            break;
    }

    if (tud_hid_ready()) {
        tud_hid_n_report(instance, 0x00, &data, sizeof(data));
    }
}

void main(void) {
    // LED
    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);
    led_blink_stat = add_repeating_timer_ms(1000, led_blink, NULL, &timer);

    // TEMP
    adc_init();
    adc_set_temp_sensor_enabled(true);

    tusb_init();

    while (true) {
        tud_task();
    }
}