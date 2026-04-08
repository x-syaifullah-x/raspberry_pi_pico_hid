#pragma once

#include "pico.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef union {
    uint32_t raw[2];
    struct {
        pico_id_t id;
        pico_status_t status;
        pico_cmd_t cmd;
        pico_data_t data;
    };
} pico_rx_t;

typedef union {
    uint32_t raw[2];
    struct {
        pico_id_t id;
        pico_cmd_t cmd;
        uint8_t args[6];
    };
} pico_tx_t;

#ifdef __cplusplus
}
#endif