#pragma once

#include "hal/pin.h"
#include "hardware/clocks.h"
#include "pico/types.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOTOR_PWM_WRAP 1000
#define MOTOR_PWM_TARGET_FREQ_KHZ 1.0F
#define MOTOR_PWM_CLKDIV ((float)clock_get_hz(clk_sys) / (MOTOR_PWM_TARGET_FREQ_KHZ * 1000.0f) / (MOTOR_PWM_WRAP + 1))
#define MOTOR_PWM_DUTY(speed) (((speed) > 100 ? 100 : (speed)) * 10)

typedef enum {
    MOTOR_RESULT_OK = 0,
    MOTOR_RESULT_ERR,
} motor_result_t;

typedef enum __attribute__((packed)) {
    MOTOR_DIRECTION_COAST = 0x00,
    MOTOR_DIRECTION_FORWARD,
    MOTOR_DIRECTION_REVERSE,
    MOTOR_DIRECTION_BRAKE,
} motor_direction_t;

typedef struct motor_driver motor_driver_t;

typedef struct {
    motor_result_t (*init)(motor_driver_t* driver);
    motor_result_t (*deinit)(motor_driver_t* driver);
    motor_result_t (*set_direction)(motor_driver_t* driver, motor_direction_t direction);
    motor_result_t (*set_speed)(motor_driver_t* driver, uint8_t speed);
} motor_driver_ops_t;

typedef struct {
    motor_direction_t direction;
    uint8_t speed;
} motor_state_t;

struct motor_driver {
    const motor_driver_ops_t* ops;
    void* ctx;
    motor_state_t state;
};

typedef struct {
    pin_t gpio;
    uint slice;
    uint channel;
} motor_pwm_pin_t;

#define MOTOR_DRIVER_DEFINE_CREATE(name, ctx_type)                                \
    motor_result_t driver_##name##_create(motor_driver_t* driver, ctx_type* hw) { \
        if (!driver || !hw)                                                       \
            return MOTOR_RESULT_ERR;                                              \
        driver->ops = &ops;                                                       \
        driver->ctx = hw;                                                         \
        return MOTOR_RESULT_OK;                                                   \
    }

#ifdef __cplusplus
}
#endif