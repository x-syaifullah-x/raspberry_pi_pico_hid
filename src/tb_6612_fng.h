#include <stdint.h>

#define TB_6612_FNG_PWM_WRAP 1250    // 125MHz
#define TB_6612_FNG_PWM_CLKDIV 5.0F  // (100KHz / 5) = 20KHz

#define TB_6612_FNG_PWM_DUTY(speed) (((speed) > 100 ? 100 : (speed)) * TB_6612_FNG_PWM_WRAP / 100)

static volatile uint tb_6612_fng_slice, tb_6612_fng_chan;

static inline bool tb_6612_fng_break(uint8_t stby, uint8_t in_1, uint8_t in_2) {
    if (!gpio_get(stby))
        return false;

    pwm_set_chan_level(tb_6612_fng_slice, tb_6612_fng_chan, 0);
    gpio_put(in_1, 1);
    gpio_put(in_2, 1);
    pwm_set_chan_level(tb_6612_fng_slice, tb_6612_fng_chan, TB_6612_FNG_PWM_DUTY(100));

    return true;
}

static inline bool tb_6612_fng_reverse(uint8_t stby, uint8_t in_1, uint8_t in_2, uint8_t speed) {
    if (!gpio_get(stby))
        return false;
    pwm_set_chan_level(tb_6612_fng_slice, tb_6612_fng_chan, 0);
    gpio_put(in_1, 0);
    gpio_put(in_2, 1);
    pwm_set_chan_level(tb_6612_fng_slice, tb_6612_fng_chan, TB_6612_FNG_PWM_DUTY(speed));

    return true;
}

static inline bool tb_6612_fng_forward(uint8_t stby, uint8_t in_1, uint8_t in_2, uint8_t speed) {
    if (!gpio_get(stby))
        return false;
    pwm_set_chan_level(tb_6612_fng_slice, tb_6612_fng_chan, 0);
    gpio_put(in_1, 1);
    gpio_put(in_2, 0);
    pwm_set_chan_level(tb_6612_fng_slice, tb_6612_fng_chan, TB_6612_FNG_PWM_DUTY(speed));
    return true;
}

static inline bool tb_6612_fng_free(uint8_t stby, uint8_t in_1, uint8_t in_2) {
    if (!gpio_get(stby))
        return false;

    gpio_put(in_1, 0);
    gpio_put(in_2, 0);
    pwm_set_chan_level(tb_6612_fng_slice, tb_6612_fng_chan, 0);

    return true;
}

static inline void tb_6612_fng_off(uint8_t stby, uint8_t in_1, uint8_t in_2, uint8_t pwm_pin) {
    gpio_deinit(in_1);
    gpio_deinit(in_2);

    pwm_set_chan_level(tb_6612_fng_slice, tb_6612_fng_chan, 0);
    pwm_set_enabled(tb_6612_fng_slice, false);
    pwm_set_counter(tb_6612_fng_slice, 0);
    gpio_set_function(pwm_pin, GPIO_FUNC_SIO);
    gpio_set_dir(pwm_pin, GPIO_IN);

    gpio_deinit(stby);
}

static inline bool tb_6612_fng_on(uint8_t stby, uint8_t in_1, uint8_t in_2, uint8_t pwm_pin) {
    if (gpio_get(stby))
        return false;

    gpio_init(stby);
    gpio_set_dir(stby, GPIO_OUT);
    gpio_put(stby, 1);

    gpio_init(in_1);
    gpio_init(in_2);

    gpio_set_dir(in_1, GPIO_OUT);
    gpio_set_dir(in_2, GPIO_OUT);

    gpio_set_function(pwm_pin, GPIO_FUNC_PWM);
    tb_6612_fng_slice = pwm_gpio_to_slice_num(pwm_pin);
    tb_6612_fng_chan = pwm_gpio_to_channel(pwm_pin);
    pwm_set_wrap(tb_6612_fng_slice, TB_6612_FNG_PWM_WRAP);
    pwm_set_clkdiv(tb_6612_fng_slice, TB_6612_FNG_PWM_CLKDIV);
    pwm_set_enabled(tb_6612_fng_slice, true);
}