#include "driver_motor_zk5ad.h"

#include "hardware/gpio.h"
#include "hardware/pwm.h"

static motor_result_t init(motor_driver_t* driver) {
    zk5ad_t* hw = driver->ctx;

    gpio_set_function(hw->pwm_a.gpio, GPIO_FUNC_PWM);
    gpio_set_function(hw->pwm_b.gpio, GPIO_FUNC_PWM);

    hw->pwm_a.slice = pwm_gpio_to_slice_num(hw->pwm_a.gpio);
    hw->pwm_a.channel = pwm_gpio_to_channel(hw->pwm_a.gpio);

    hw->pwm_b.slice = pwm_gpio_to_slice_num(hw->pwm_b.gpio);
    hw->pwm_b.channel = pwm_gpio_to_channel(hw->pwm_b.gpio);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, MOTOR_PWM_WRAP);
    pwm_config_set_clkdiv(&cfg, MOTOR_PWM_CLKDIV);
    pwm_init(hw->pwm_a.slice, &cfg, true);
    if (hw->pwm_b.slice != hw->pwm_a.slice)
        pwm_init(hw->pwm_b.slice, &cfg, true);

    driver->state.direction = MOTOR_DIRECTION_COAST;
    driver->state.speed = 0;

    return MOTOR_RESULT_OK;
}

static motor_result_t deinit(motor_driver_t* driver) {
    zk5ad_t* hw = (zk5ad_t*)driver->ctx;

    driver->state.direction = MOTOR_DIRECTION_COAST;
    driver->state.speed = 0;

    pwm_set_enabled(hw->pwm_a.slice, false);
    if (hw->pwm_b.slice != hw->pwm_a.slice)
        pwm_set_enabled(hw->pwm_b.slice, false);

    gpio_set_function(hw->pwm_a.gpio, GPIO_FUNC_SIO);
    gpio_set_function(hw->pwm_b.gpio, GPIO_FUNC_SIO);

    gpio_set_dir(hw->pwm_a.gpio, GPIO_IN);
    gpio_set_dir(hw->pwm_b.gpio, GPIO_IN);

    return MOTOR_RESULT_OK;
}

static motor_result_t set_direction(motor_driver_t* driver, motor_direction_t direction) {
    const zk5ad_t* hw = (zk5ad_t*)driver->ctx;
    switch (direction) {
        case MOTOR_DIRECTION_COAST:
            pwm_set_chan_level(hw->pwm_a.slice, hw->pwm_a.channel, 0);
            pwm_set_chan_level(hw->pwm_b.slice, hw->pwm_b.channel, 0);
            break;
        case MOTOR_DIRECTION_FORWARD:
            pwm_set_chan_level(hw->pwm_a.slice, hw->pwm_a.channel, MOTOR_PWM_DUTY(driver->state.speed));
            pwm_set_chan_level(hw->pwm_b.slice, hw->pwm_b.channel, 0);
            break;
        case MOTOR_DIRECTION_REVERSE:
            pwm_set_chan_level(hw->pwm_a.slice, hw->pwm_a.channel, 0);
            pwm_set_chan_level(hw->pwm_b.slice, hw->pwm_b.channel, MOTOR_PWM_DUTY(driver->state.speed));
            break;
        case MOTOR_DIRECTION_BRAKE:
            pwm_set_chan_level(hw->pwm_a.slice, hw->pwm_a.channel, MOTOR_PWM_DUTY(driver->state.speed));
            pwm_set_chan_level(hw->pwm_b.slice, hw->pwm_b.channel, MOTOR_PWM_DUTY(driver->state.speed));
            break;
        default:
            return MOTOR_RESULT_ERR;
    }

    driver->state.direction = direction;

    return MOTOR_RESULT_OK;
}

static motor_result_t set_speed(motor_driver_t* driver, uint8_t speed) {
    zk5ad_t* hw = (zk5ad_t*)driver->ctx;
    switch (driver->state.direction) {
        case MOTOR_DIRECTION_COAST:
            pwm_set_chan_level(hw->pwm_a.slice, hw->pwm_a.channel, 0);
            pwm_set_chan_level(hw->pwm_b.slice, hw->pwm_b.channel, 0);
            break;
        case MOTOR_DIRECTION_FORWARD:
            pwm_set_chan_level(hw->pwm_a.slice, hw->pwm_a.channel, MOTOR_PWM_DUTY(speed));
            pwm_set_chan_level(hw->pwm_b.slice, hw->pwm_b.channel, 0);
            break;
        case MOTOR_DIRECTION_REVERSE:
            pwm_set_chan_level(hw->pwm_a.slice, hw->pwm_a.channel, 0);
            pwm_set_chan_level(hw->pwm_b.slice, hw->pwm_b.channel, MOTOR_PWM_DUTY(speed));
            break;
        case MOTOR_DIRECTION_BRAKE:
            pwm_set_chan_level(hw->pwm_a.slice, hw->pwm_a.channel, MOTOR_PWM_DUTY(speed));
            pwm_set_chan_level(hw->pwm_b.slice, hw->pwm_b.channel, MOTOR_PWM_DUTY(speed));
            break;
        default:
            return MOTOR_RESULT_ERR;
    }

    driver->state.speed = speed;

    return MOTOR_RESULT_OK;
}

static const motor_driver_ops_t ops = {
    .init = init,
    .deinit = deinit,
    .set_direction = set_direction,
    .set_speed = set_speed,
};

MOTOR_DRIVER_DEFINE_CREATE(zk5ad, zk5ad_t)

// motor_result_t driver_zk5ad_create(motor_driver_t* driver, zk5ad_t* hw) {
//     if (!driver || !hw)
//         return MOTOR_ERR;

//     static const motor_driver_ops_t zk5ad_ops = {
//         .init = init,
//         .deinit = deinit,
//         .set_direction = set_direction,
//         .set_speed = set_speed,
//     };

//     driver->ops = &zk5ad_ops;
//     driver->ctx = hw;

//     return MOTOR_OK;
// }