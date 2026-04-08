#include "hardware/adc.h"
#include "hardware/pwm.h"
#include "hardware/watchdog.h"
#include "hid_report.h"
#include "pico/bootrom.h"
#include "pico/stdlib.h"
#include "pin.h"
#include "tusb.h"

#define METHODE_GET_GPIO 0x00
#define METHODE_SET_DEFAULT_LED 0x01
#define METHODE_SYSTEM_READ_TEMP 0x02
#define METHODE_TB6612FNG 0x03
#define METHODE_TB6612FNG_POWER 0x00
#define METHODE_TB6612FNG_POWER_OFF 0x00
#define METHODE_TB6612FNG_POWER_ON 0x01
#define METHODE_TB6612FNG_MOTOR 0x01
#define METHODE_TB6612FNG_MOTOR_1 0x01
#define METHODE_TB6612FNG_MOTOR_2 0x02
#define METHODE_SYSTEM 0xFF
#define METHODE_SYSTEM_REBOOT 0x00
#define METHODE_SYSTEM_REBOOT_BOOTSEL 0x01

#define INVALID_METHODE_NUMBER 0x1
#define INVALID_METHOD_PARAM 0x2

#define MOTOR_PWM_WRAP 1250    // 125MHz
#define MOTOR_PWM_CLKDIV 5.0f  // (100KHz / 5) = 20KHz

volatile bool led_blink_stat = false;
volatile uint slice, chan;

struct repeating_timer timer;

static inline bool led_blink(struct repeating_timer *t) {
    static bool state = false;
    gpio_put(PICO_DEFAULT_LED_PIN, (state = !state));
    return true;
}

void tud_hid_set_report_cb(uint8_t instance, uint8_t report_id, hid_report_type_t report_type, uint8_t const *params, uint16_t param_size) {
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
                    led_blink_stat = add_repeating_timer_ms(((param_size >= 2) ? (params[1] * 100) : 1000), led_blink, NULL, &timer);
                    data.value = 0x02;
                    break;
                default:
                    data.err = INVALID_METHOD_PARAM;
                    break;
            }
            break;
        case METHODE_SYSTEM_READ_TEMP:
            adc_select_input(4);
            uint16_t raw /* 0 – 4095 */ = adc_read();
            data.value = raw >> 4;
            // float voltage = raw * 3.3f / 4095;
            // gpio.value = 27 - (voltage - 0.706f) / 0.001721f;
            break;
        case METHODE_TB6612FNG:
            switch (params[0]) {
                // printf '\x03\x00\x[00(OFF) || 01(ON)]' > /dev/hidrawN
                case METHODE_TB6612FNG_POWER:
                    switch (params[1]) {
                        case METHODE_TB6612FNG_POWER_OFF:
                            gpio_deinit(PIN_01);
                            gpio_deinit(PIN_02);

                            pwm_set_chan_level(slice, chan, 0);
                            pwm_set_enabled(slice, false);
                            pwm_set_counter(slice, 0);
                            gpio_set_function(PIN_04, GPIO_FUNC_SIO);
                            gpio_set_dir(PIN_04, GPIO_IN);

                            gpio_deinit(PIN_05);

                            data.value = 0x00;
                            break;
                        case METHODE_TB6612FNG_POWER_ON:
                            if (gpio_get(PIN_05)) {
                                data.value = 0x01;
                                break;
                            }
                            gpio_init(PIN_05);
                            gpio_set_dir(PIN_05, GPIO_OUT);
                            gpio_put(PIN_05, 1);

                            gpio_init(PIN_01);
                            gpio_init(PIN_02);

                            gpio_set_dir(PIN_01, GPIO_OUT);
                            gpio_set_dir(PIN_02, GPIO_OUT);

                            gpio_set_function(PIN_04, GPIO_FUNC_PWM);
                            slice = pwm_gpio_to_slice_num(PIN_04);
                            chan = pwm_gpio_to_channel(PIN_04);
                            pwm_set_wrap(slice, MOTOR_PWM_WRAP);
                            pwm_set_clkdiv(slice, MOTOR_PWM_CLKDIV);
                            pwm_set_enabled(slice, true);
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
                                    if (!gpio_get(PIN_05)) {
                                        data.err = INVALID_METHOD_PARAM;
                                        break;
                                    }
                                    pwm_set_chan_level(slice, chan, 0);
                                    gpio_put(PIN_01, 0);
                                    gpio_put(PIN_02, 0);
                                    break;
                                case 0x01:  // FORWARD
                                    if (!gpio_get(PIN_05)) {
                                        data.err = INVALID_METHOD_PARAM;
                                        break;
                                    }
                                    pwm_set_chan_level(slice, chan, (params[3] * MOTOR_PWM_WRAP) / 100);
                                    gpio_put(PIN_01, 1);
                                    gpio_put(PIN_02, 0);
                                    break;
                                case 0x02:  // REVERSE
                                    if (!gpio_get(PIN_05)) {
                                        data.err = INVALID_METHOD_PARAM;
                                        break;
                                    }
                                    pwm_set_chan_level(slice, chan, (params[3] * MOTOR_PWM_WRAP) / 100);
                                    gpio_put(PIN_01, 0);
                                    gpio_put(PIN_02, 1);
                                    break;
                                case 0x03:  // BREAK
                                    if (!gpio_get(PIN_05)) {
                                        data.err = INVALID_METHOD_PARAM;
                                        break;
                                    }
                                    pwm_set_chan_level(slice, chan, 0);
                                    gpio_put(PIN_01, 1);
                                    gpio_put(PIN_02, 1);
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
        tud_hid_n_report(instance, 0x00, &data, sizeof(pi_pico_data));
    }
}

uint8_t const *tud_hid_descriptor_report_cb(uint8_t instance) { return hid_report_desc; }

void main(void) {
    // stdio_init_all();

    adc_init();
    adc_set_temp_sensor_enabled(true);

    gpio_init(PICO_DEFAULT_LED_PIN);
    gpio_set_dir(PICO_DEFAULT_LED_PIN, GPIO_OUT);

    tusb_init();

    led_blink_stat = add_repeating_timer_ms(1000, led_blink, NULL, &timer);

    while (true) {
        tud_task();
    }
}