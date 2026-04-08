#pragma once

#include "driver_motor.h"
#include "hal/pin.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    pin_t stby;
    pin_t in1;
    pin_t in2;
    motor_pwm_pin_t pwm;
} tb6612fng_t;

motor_result_t driver_tb6612fng_create(motor_driver_t* driver, tb6612fng_t* hw);

#ifdef __cplusplus
}
#endif