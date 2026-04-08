#ifndef _PI_PICO_DATA_H_
#define _PI_PICO_DATA_H_

#include <stdint.h>

typedef struct __attribute__((packed)) {
    uint8_t err;
    uint8_t methode;
    uint8_t gpio;
    uint16_t value;
} pi_pico_data;

#endif