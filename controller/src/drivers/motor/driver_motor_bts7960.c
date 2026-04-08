#include "driver_motor_bts7960.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"

static void apply(motor_driver_t* driver) {
    const bts7960_t* hw = (bts7960_t*)driver->ctx;
    const uint16_t duty = MOTOR_PWM_DUTY(driver->state.speed);

    switch (driver->state.direction) {
        case MOTOR_DIRECTION_FORWARD:
            pwm_set_chan_level(hw->rpwm.slice, hw->rpwm.channel, duty);
            pwm_set_chan_level(hw->lpwm.slice, hw->lpwm.channel, 0);
            break;
        case MOTOR_DIRECTION_REVERSE:
            pwm_set_chan_level(hw->rpwm.slice, hw->rpwm.channel, 0);
            pwm_set_chan_level(hw->lpwm.slice, hw->lpwm.channel, duty);
            break;
        case MOTOR_DIRECTION_BRAKE:
            pwm_set_chan_level(hw->rpwm.slice, hw->rpwm.channel, MOTOR_PWM_WRAP);
            pwm_set_chan_level(hw->lpwm.slice, hw->lpwm.channel, MOTOR_PWM_WRAP);
            break;
        case MOTOR_DIRECTION_COAST:
        default:
            pwm_set_chan_level(hw->rpwm.slice, hw->rpwm.channel, 0);
            pwm_set_chan_level(hw->lpwm.slice, hw->lpwm.channel, 0);
            break;
    }
}

static void setup_pwm_pin(motor_pwm_pin_t* pin) {
    gpio_set_function(pin->gpio, GPIO_FUNC_PWM);
    pin->slice = pwm_gpio_to_slice_num(pin->gpio);
    pin->channel = pwm_gpio_to_channel(pin->gpio);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, MOTOR_PWM_WRAP);
    pwm_config_set_clkdiv(&cfg, MOTOR_PWM_CLKDIV);
    pwm_init(pin->slice, &cfg, true);
}

static motor_result_t init(motor_driver_t* driver) {
    bts7960_t* hw = (bts7960_t*)driver->ctx;

    if (hw->r_en != (uint8_t)-1) {
        gpio_init(hw->r_en);
        gpio_set_dir(hw->r_en, GPIO_OUT);
        gpio_put(hw->r_en, 1);
    }

    if (hw->l_en != (uint8_t)-1) {
        gpio_init(hw->l_en);
        gpio_set_dir(hw->l_en, GPIO_OUT);
        gpio_put(hw->l_en, 1);
    }

    setup_pwm_pin(&hw->rpwm);
    if (hw->lpwm.gpio != hw->rpwm.gpio)
        setup_pwm_pin(&hw->lpwm);

    driver->state.direction = MOTOR_DIRECTION_COAST;
    driver->state.speed = 0;
    apply(driver);

    return MOTOR_RESULT_OK;
}

static motor_result_t deinit(motor_driver_t* driver) {
    bts7960_t* hw = (bts7960_t*)driver->ctx;

    driver->state.direction = MOTOR_DIRECTION_COAST;
    driver->state.speed = 0;
    apply(driver);

    if (hw->r_en != (uint8_t)-1) {
        gpio_put(hw->r_en, 0);
        gpio_set_dir(hw->r_en, GPIO_IN);
    }

    if (hw->l_en != (uint8_t)-1) {
        gpio_put(hw->l_en, 0);
        gpio_set_dir(hw->l_en, GPIO_IN);
    }

    pwm_set_enabled(hw->rpwm.slice, false);
    if (hw->lpwm.slice != hw->rpwm.slice)
        pwm_set_enabled(hw->lpwm.slice, false);

    gpio_set_function(hw->rpwm.gpio, GPIO_FUNC_SIO);
    gpio_set_dir(hw->rpwm.gpio, GPIO_IN);

    gpio_set_function(hw->lpwm.gpio, GPIO_FUNC_SIO);
    gpio_set_dir(hw->lpwm.gpio, GPIO_IN);

    return MOTOR_RESULT_OK;
}

static motor_result_t set_direction(motor_driver_t* driver, motor_direction_t direction) {
    if (direction > MOTOR_DIRECTION_BRAKE) {
        return MOTOR_RESULT_ERR;
    }
    driver->state.direction = direction;
    apply(driver);
    return MOTOR_RESULT_OK;
}

static motor_result_t set_speed(motor_driver_t* driver, uint8_t speed) {
    driver->state.speed = speed;
    apply(driver);
    return MOTOR_RESULT_OK;
}

static const motor_driver_ops_t ops = {
    .init = init,
    .deinit = deinit,
    .set_direction = set_direction,
    .set_speed = set_speed,
};

MOTOR_DRIVER_DEFINE_CREATE(bts7960, bts7960_t)

// motor_result_t driver_bts7960_create(motor_driver_t* driver, bts7960_t* hw) {
//     static const motor_driver_ops_t ops = {
//         .init = init,
//         .deinit = deinit,
//         .set_direction = set_direction,
//         .set_speed = set_speed,
//     };

//     if (!driver || !hw)
//         return MOTOR_ERR;

//     driver->ops = &ops;
//     driver->ctx = hw;

//     return MOTOR_OK;
// }