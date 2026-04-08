#pragma once

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    STATUS_OK = 0x00,
    STATUS_ERR = 0x01,
} status_t;

typedef enum __attribute__((packed)) {
    STATUS_ERR_CMD_UNKNOWN = 0x00,
    STATUS_ERR_CMD_NOT_IMPLEMENTED,
    STATUS_ERR_CMD_ARG_INVALID,
    STATUS_ERR_CMD_ARG_SIZE_INVALID,
} status_err_t;

typedef enum __attribute__((packed)) {
    // CMD_UNKNOWN = 0,
    CMD_SYSTEM_REBOOT = 1,
    CMD_SYSTEM_RESET_USB,
    CMD_SYSTEM_MEMORY,
    CMD_SYSTEM_END = 19,

    CMD_LED_BEGIN = 20,
    CMD_LED_DEFAULT,
    CMD_LED_WS2812_1BIT,
    CMD_LED_END = 29,

    CMD_ADC_BEGIN = 40,
    CMD_ADC_DMA,
    CMD_ADC_READ_CH0,
    CMD_ADC_READ_CH1,
    CMD_ADC_READ_CH2,
    CMD_ADC_READ_CH3,
    CMD_ADC_READ_CH4,
    CMD_ADC_END = 59,

    CMD_MOTOR_BEGIN = 60,
    CMD_MOTOR,
    CMD_MOTOR_END = 69,
} cmd_t;

typedef enum __attribute__((packed)) {
    ID_DEVICE = 0x00,
    ID_HOST = 0x01,
} id_t;

#ifdef __cplusplus
}
#endif
