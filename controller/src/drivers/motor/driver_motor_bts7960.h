#pragma once

#include "driver_motor.h"
#include "hal/pin.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    pin_t r_en;
    pin_t l_en;
    motor_pwm_pin_t rpwm;
    motor_pwm_pin_t lpwm;
} bts7960_t;

motor_result_t driver_bts7960_create(motor_driver_t* driver, bts7960_t* hw);

#ifdef __cplusplus
}
#endif