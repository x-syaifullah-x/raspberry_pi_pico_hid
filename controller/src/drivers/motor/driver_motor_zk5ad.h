#pragma once

#include "driver_motor.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct {
    motor_pwm_pin_t pwm_a;
    motor_pwm_pin_t pwm_b;
} zk5ad_t;

motor_result_t driver_zk5ad_create(motor_driver_t* driver, zk5ad_t* hw);

#ifdef __cplusplus
}
#endif