#pragma once

#include "gpio_num.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    PIN_01 = GPIO_NUM_0,
    PIN_02 = GPIO_NUM_1,
    PIN_04 = GPIO_NUM_2,
    PIN_05 = GPIO_NUM_3,
    PIN_06 = GPIO_NUM_4,
    PIN_07 = GPIO_NUM_5,
    PIN_09 = GPIO_NUM_6,
    PIN_10 = GPIO_NUM_7,
    PIN_11 = GPIO_NUM_8,
    PIN_12 = GPIO_NUM_9,
    PIN_14 = GPIO_NUM_10,
    PIN_15 = GPIO_NUM_11,
    PIN_16 = GPIO_NUM_12,
    PIN_17 = GPIO_NUM_13,
    PIN_19 = GPIO_NUM_14,
    PIN_20 = GPIO_NUM_15,
    PIN_21 = GPIO_NUM_16,
    PIN_22 = GPIO_NUM_17,
    PIN_24 = GPIO_NUM_18,
    PIN_25 = GPIO_NUM_19,
    PIN_26 = GPIO_NUM_20,
    PIN_27 = GPIO_NUM_21,
    PIN_29 = GPIO_NUM_22,
    PIN_31 = GPIO_NUM_26,
    PIN_32 = GPIO_NUM_27,
    PIN_34 = GPIO_NUM_28,
    PIN_MAX,
} pin_t;

#ifdef __cplusplus
}
#endif