#pragma once

#include "driver_motor.h"
#include "hal/pin.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    pin_t stby;
    motor_pwm_pin_t in1;
    motor_pwm_pin_t in2;
} drv8833_t;

motor_result_t driver_drv8833_create(motor_driver_t* driver, drv8833_t* hw);

#ifdef __cplusplus
}
#endif