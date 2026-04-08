#include "driver_motor_tb6612fng.h"

#include "hal/pin.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

static uint8_t tb6612fng_stby_refcount[PIN_MAX];

static void apply(motor_driver_t* driver) {
    const tb6612fng_t* hw = (tb6612fng_t*)driver->ctx;
    const uint16_t duty = MOTOR_PWM_DUTY(driver->state.speed);

    switch (driver->state.direction) {
        case MOTOR_DIRECTION_FORWARD:
            gpio_put(hw->in1, 1);
            gpio_put(hw->in2, 0);
            pwm_set_chan_level(hw->pwm.slice, hw->pwm.channel, duty);
            break;
        case MOTOR_DIRECTION_REVERSE:
            gpio_put(hw->in1, 0);
            gpio_put(hw->in2, 1);
            pwm_set_chan_level(hw->pwm.slice, hw->pwm.channel, duty);
            break;
        case MOTOR_DIRECTION_BRAKE:
            gpio_put(hw->in1, 1);
            gpio_put(hw->in2, 1);
            pwm_set_chan_level(hw->pwm.slice, hw->pwm.channel, duty);
            break;
        case MOTOR_DIRECTION_COAST:
        default:
            gpio_put(hw->in1, 0);
            gpio_put(hw->in2, 0);
            pwm_set_chan_level(hw->pwm.slice, hw->pwm.channel, 0);
            break;
    }
}

static motor_result_t init(motor_driver_t* driver) {
    tb6612fng_t* hw = (tb6612fng_t*)driver->ctx;

    if (hw->stby != (uint8_t)-1) {
        gpio_init(hw->stby);
        gpio_set_dir(hw->stby, GPIO_OUT);
        if (tb6612fng_stby_refcount[hw->stby] == 0) {
            gpio_init(hw->stby);
            gpio_set_dir(hw->stby, GPIO_OUT);
            gpio_put(hw->stby, 1);
        }
        tb6612fng_stby_refcount[hw->stby]++;
    }

    gpio_init(hw->in1);
    gpio_set_dir(hw->in1, GPIO_OUT);

    gpio_init(hw->in2);
    gpio_set_dir(hw->in2, GPIO_OUT);

    gpio_set_function(hw->pwm.gpio, GPIO_FUNC_PWM);
    hw->pwm.slice = pwm_gpio_to_slice_num(hw->pwm.gpio);
    hw->pwm.channel = pwm_gpio_to_channel(hw->pwm.gpio);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, MOTOR_PWM_WRAP);
    pwm_config_set_clkdiv(&cfg, MOTOR_PWM_CLKDIV);
    pwm_init(hw->pwm.slice, &cfg, true);

    driver->state.direction = MOTOR_DIRECTION_COAST;
    driver->state.speed = 0;

    return MOTOR_RESULT_OK;
}

static motor_result_t deinit(motor_driver_t* driver) {
    tb6612fng_t* hw = (tb6612fng_t*)driver->ctx;

    driver->state.direction = MOTOR_DIRECTION_COAST;
    driver->state.speed = 0;

    apply(driver);

    if (hw->stby != (uint8_t)-1) {
        if (tb6612fng_stby_refcount[hw->stby])
            tb6612fng_stby_refcount[hw->stby]--;

        if (tb6612fng_stby_refcount[hw->stby] == 0)
            gpio_put(hw->stby, 0);
    }

    pwm_set_enabled(hw->pwm.slice, false);

    gpio_set_function(hw->in1, GPIO_FUNC_SIO);
    gpio_set_function(hw->in2, GPIO_FUNC_SIO);
    gpio_set_function(hw->pwm.gpio, GPIO_FUNC_SIO);

    gpio_set_dir(hw->in1, GPIO_IN);
    gpio_set_dir(hw->in2, GPIO_IN);
    gpio_set_dir(hw->pwm.gpio, GPIO_IN);

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

MOTOR_DRIVER_DEFINE_CREATE(tb6612fng, tb6612fng_t)

// motor_result_t driver_tb6612fng_create(motor_driver_t* driver, tb6612fng_t* hw) {
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