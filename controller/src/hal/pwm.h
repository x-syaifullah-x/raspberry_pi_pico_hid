#pragma once

#include "hardware/pwm.h"

#ifdef __cplusplus
extern "C" {
#endif

#define PWM_WRAP 1000
#define PWM_CLKDIV 6.25F
#define PWM_DUTY(speed) (((speed) > 100 ? 100 : (speed)) * 10)

static inline uint32_t get_pwm_level(uint slice_num, uint channel) {
    return (channel == PWM_CHAN_A)
               ? (pwm_hw->slice[slice_num].cc & 0xFFFF)
               : ((pwm_hw->slice[slice_num].cc >> 16) & 0xFFFF);
}

#ifdef __cplusplus
}
#endif