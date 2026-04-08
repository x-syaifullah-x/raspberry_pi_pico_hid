#include <stdint.h>

#ifndef _PIN_H_
#define _PIN_H_

typedef struct __attribute__((packed)) {
    uint8_t err;
    uint8_t methode;
    uint8_t gpio;
    uint8_t value;
} pi_pico_data;

#endif