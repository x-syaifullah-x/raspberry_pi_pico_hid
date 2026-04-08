#include "driver_motor_drv8833.h"

#include "hal/pin.h"
#include "hardware/gpio.h"
#include "hardware/pwm.h"

static uint8_t drv8833_stby_refcount[PIN_MAX];

static void apply(motor_driver_t* driver) {
    const drv8833_t* hw = (drv8833_t*)driver->ctx;
    const uint16_t duty = MOTOR_PWM_DUTY(driver->state.speed);

    switch (driver->state.direction) {
        case MOTOR_DIRECTION_FORWARD:
            pwm_set_chan_level(hw->in1.slice, hw->in1.channel, duty);
            pwm_set_chan_level(hw->in2.slice, hw->in2.channel, 0);
            break;
        case MOTOR_DIRECTION_REVERSE:
            pwm_set_chan_level(hw->in1.slice, hw->in1.channel, 0);
            pwm_set_chan_level(hw->in2.slice, hw->in2.channel, duty);
            break;
        case MOTOR_DIRECTION_BRAKE:
            pwm_set_chan_level(hw->in1.slice, hw->in1.channel, duty);
            pwm_set_chan_level(hw->in2.slice, hw->in2.channel, duty);
            break;
        case MOTOR_DIRECTION_COAST:
        default:
            pwm_set_chan_level(hw->in1.slice, hw->in1.channel, 0);
            pwm_set_chan_level(hw->in2.slice, hw->in2.channel, 0);
            break;
    }
}

static motor_result_t init(motor_driver_t* driver) {
    drv8833_t* hw = (drv8833_t*)driver->ctx;

    if (hw->stby != (uint8_t)-1) {
        gpio_init(hw->stby);
        gpio_set_dir(hw->stby, GPIO_OUT);
        if (drv8833_stby_refcount[hw->stby] == 0) {
            gpio_init(hw->stby);
            gpio_set_dir(hw->stby, GPIO_OUT);
            gpio_put(hw->stby, 1);
        }
        drv8833_stby_refcount[hw->stby]++;
    }

    gpio_set_function(hw->in1.gpio, GPIO_FUNC_PWM);
    hw->in1.slice = pwm_gpio_to_slice_num(hw->in1.gpio);
    hw->in1.channel = pwm_gpio_to_channel(hw->in1.gpio);

    gpio_set_function(hw->in2.gpio, GPIO_FUNC_PWM);
    hw->in2.slice = pwm_gpio_to_slice_num(hw->in2.gpio);
    hw->in2.channel = pwm_gpio_to_channel(hw->in2.gpio);

    pwm_config cfg = pwm_get_default_config();
    pwm_config_set_wrap(&cfg, MOTOR_PWM_WRAP);
    pwm_config_set_clkdiv(&cfg, MOTOR_PWM_CLKDIV);
    pwm_init(hw->in1.slice, &cfg, true);
    if (hw->in2.slice != hw->in1.slice)
        pwm_init(hw->in2.slice, &cfg, true);

    driver->state.direction = MOTOR_DIRECTION_COAST;
    driver->state.speed = 0;

    return MOTOR_RESULT_OK;
}

static motor_result_t deinit(motor_driver_t* driver) {
    drv8833_t* hw = (drv8833_t*)driver->ctx;

    driver->state.direction = MOTOR_DIRECTION_COAST;
    driver->state.speed = 0;

    apply(driver);

    if (hw->stby != (uint8_t)-1) {
        if (drv8833_stby_refcount[hw->stby])
            drv8833_stby_refcount[hw->stby]--;

        if (drv8833_stby_refcount[hw->stby] == 0)
            gpio_put(hw->stby, 0);
    }

    pwm_set_enabled(hw->in1.slice, false);
    if (hw->in2.slice != hw->in1.slice)
        pwm_set_enabled(hw->in2.slice, false);

    gpio_set_function(hw->in1.gpio, GPIO_FUNC_SIO);
    gpio_set_function(hw->in2.gpio, GPIO_FUNC_SIO);

    gpio_set_dir(hw->in1.gpio, GPIO_IN);
    gpio_set_dir(hw->in2.gpio, GPIO_IN);

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

motor_result_t driver_drv8833_create(motor_driver_t* driver, drv8833_t* hw) {
    static const motor_driver_ops_t ops = {
        .init = init,
        .deinit = deinit,
        .set_direction = set_direction,
        .set_speed = set_speed,
    };

    if (!driver || !hw)
        return MOTOR_RESULT_ERR;

    driver->ops = &ops;
    driver->ctx = hw;

    return MOTOR_RESULT_OK;
}