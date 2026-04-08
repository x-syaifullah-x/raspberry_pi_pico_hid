#pragma once

#include <type/pico.h>

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) {
    const pico_cmd_t cmd;
    pico_data_t data;
} ws_state_t;

static ws_state_t pico_states[] = {
    {.cmd = PICO_CMD_LED_DEFAULT, .data = {0}},
    {.cmd = PICO_CMD_ADC_DMA, .data = {0}},
    {.cmd = PICO_CMD_ADC_CH0, .data = {0}},
    {.cmd = PICO_CMD_ADC_CH1, .data = {0}},
    {.cmd = PICO_CMD_ADC_CH2, .data = {0}},
    {.cmd = PICO_CMD_ADC_CH4, .data = {0}},
    {.cmd = PICO_CMD_MOTOR_1, .data = {0}},
    {.cmd = PICO_CMD_MOTOR_2, .data = {0}},
    {.cmd = PICO_CMD_MOTOR_3, .data = {0}},
};

static ws_state_t* pico_state_map[PICO_CMD_END] = {
    [PICO_CMD_LED_DEFAULT] = &pico_states[0],
    [PICO_CMD_ADC_DMA] = &pico_states[1],
    [PICO_CMD_ADC_CH0] = &pico_states[2],
    [PICO_CMD_ADC_CH1] = &pico_states[3],
    [PICO_CMD_ADC_CH2] = &pico_states[4],
    [PICO_CMD_ADC_CH4] = &pico_states[5],
    [PICO_CMD_MOTOR_1] = &pico_states[6],
    [PICO_CMD_MOTOR_2] = &pico_states[7],
    [PICO_CMD_MOTOR_3] = &pico_states[8],
};

#ifdef __cplusplus
}
#endif