#pragma once

#include "hal/pin.h"
#include "hardware/adc.h"
#include "hardware/dma.h"
#include "hardware/irq.h"
#include "packet/rx/packet_rx.h"
#include "packet/rx/packet_rx_rb.h"
#include "packet/tx/packet_tx.h"
#include "pico/time.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    ADC_CH_0 = 0x00,  // GPIO26 (PIN_31)
    ADC_CH_1,         // GPIO27 (PIN_32)
    ADC_CH_2,         // GPIO28 (PIN_34)
    ADC_CH_3,         // GPIO29
    ADC_CH_4,         // TEMP SENSOR INTERNAL
} adc_input_t;

static const uint32_t ADC_CHANNELS[] = {ADC_CH_0, ADC_CH_1, ADC_CH_2};

#define NUM_ADC_CH (sizeof(ADC_CHANNELS) / sizeof(ADC_CHANNELS[0]))
#define SAMPLES_PER_CH 256
#define SAMPLES_SHIFT 8
#define BUFFER_SIZE (NUM_ADC_CH * SAMPLES_PER_CH)

#define TEMP_DISCARD 32
#define TEMP_SAMPLES 16
#define TEMP_SHIFT 4
#define TEMP_TOTAL (TEMP_DISCARD + TEMP_SAMPLES)

static uint32_t adc_rr_mask(void) {
    uint32_t mask = 0;
    for (uint32_t i = 0; i < NUM_ADC_CH; i++)
        mask |= (1U << (ADC_CHANNELS[i]));
    return mask;
}

static volatile uint16_t ch_average[5];

static uint16_t buf[2][BUFFER_SIZE];
static uint16_t temp_buf[2][TEMP_TOTAL];
static volatile uint32_t write_idx = 0;
static volatile uint32_t read_idx = 1;
static volatile bool capture_temp = false;
static int dma_chan = -1;
static dma_channel_config dma_cfg;

void dma_irq_handler(void) {
    dma_hw->ints0 = 1U << dma_chan;
    adc_run(false);

    read_idx = write_idx;
    write_idx ^= 1;

    if (!capture_temp) {
        for (uint32_t i = 0; i < NUM_ADC_CH; i++) {
            uint32_t sum = 0;
            const uint16_t* p = &buf[read_idx][i];
            for (uint32_t s = 0; s < SAMPLES_PER_CH; s++, p += NUM_ADC_CH)
                sum += *p;
            ch_average[ADC_CHANNELS[i]] = sum >> SAMPLES_SHIFT;
        }

        // Switch to ADC4 (temp)
        capture_temp = true;
        adc_set_round_robin(0);
        adc_select_input(ADC_CH_4);
        adc_set_clkdiv(960.0F);

        dma_channel_set_write_addr(dma_chan, temp_buf[write_idx], false);
        dma_channel_set_trans_count(dma_chan, TEMP_TOTAL, true);
    } else {
        uint32_t sum = 0;
        const uint16_t* p = &temp_buf[read_idx][TEMP_DISCARD];
        for (uint32_t i = 0; i < TEMP_SAMPLES; i++)
            sum += *p++;
        ch_average[ADC_CH_4] = sum >> TEMP_SHIFT;

        // Switch to ADC voltage
        capture_temp = false;
        adc_set_round_robin(adc_rr_mask());
        adc_select_input(ADC_CHANNELS[0]);
        adc_set_clkdiv(0);

        dma_channel_set_write_addr(dma_chan, buf[write_idx], false);
        dma_channel_set_trans_count(dma_chan, BUFFER_SIZE, true);
    }

    adc_run(true);
}

bool adc_temp_sensor_timer_callback(repeating_timer_t* timer) {
    (void)timer;
    packet_rx_t packet_rx = {.id = ID_DEVICE};
    for (uint32_t i = 0; i < NUM_ADC_CH; i++) {
        switch (ADC_CHANNELS[i]) {
            case ADC_CH_0:
                packet_rx.cmd = CMD_ADC_READ_CH0;
                break;
            case ADC_CH_1:
                packet_rx.cmd = CMD_ADC_READ_CH1;
                break;
            case ADC_CH_2:
                packet_rx.cmd = CMD_ADC_READ_CH2;
                break;
        }
        packet_rx_rb_push(&packet_rx);
    }

    packet_rx.cmd = CMD_ADC_READ_CH4;
    packet_rx_rb_push(&packet_rx);
    return true;
}

static struct repeating_timer adc_temp_sensor_timer;
static bool adc_dma_running = false;

bool adc_dma_init(void) {
    adc_init();
    for (uint32_t i = 0; i < NUM_ADC_CH; i++) {
        uint32_t ch = ADC_CHANNELS[i];
        if (ch < 4)
            adc_gpio_init(PIN_31 + ch);
    }
    adc_set_temp_sensor_enabled(true);
    adc_set_round_robin(adc_rr_mask());
    adc_select_input(ADC_CHANNELS[0]);
    adc_fifo_setup(true, true, 1, false, false);
    adc_set_clkdiv(0);

    dma_chan = dma_claim_unused_channel(false);
    if (dma_chan == -1)
        return false;
    dma_cfg = dma_channel_get_default_config(dma_chan);
    channel_config_set_transfer_data_size(&dma_cfg, DMA_SIZE_16);
    channel_config_set_read_increment(&dma_cfg, false);
    channel_config_set_write_increment(&dma_cfg, true);
    channel_config_set_dreq(&dma_cfg, DREQ_ADC);

    dma_channel_set_irq0_enabled(dma_chan, true);
    irq_set_exclusive_handler(DMA_IRQ_0, dma_irq_handler);
    irq_set_enabled(DMA_IRQ_0, true);
    return true;
}

void adc_dma_start(void) {
    if (adc_dma_running) return;

    adc_run(false);
    while (!adc_fifo_is_empty()) adc_fifo_get();
    write_idx = 0;
    read_idx = 1;
    capture_temp = false;

    dma_channel_configure(dma_chan, &dma_cfg, buf[0], &adc_hw->fifo, BUFFER_SIZE, false);
    adc_run(true);
    dma_channel_start(dma_chan);

    add_repeating_timer_ms(1000, adc_temp_sensor_timer_callback, NULL, &adc_temp_sensor_timer);

    adc_dma_running = true;
}

void adc_dma_stop(void) {
    if (!adc_dma_running) return;

    adc_run(false);
    // dma_channel_abort(dma_chan);
    dma_hw->ch[dma_chan].ctrl_trig &= ~DMA_CH0_CTRL_TRIG_EN_BITS;
    // while (!adc_fifo_is_empty()) adc_fifo_get();

    cancel_repeating_timer(&adc_temp_sensor_timer);

    adc_dma_running = false;
}

static bool adc_dma_initialized = false;

static inline packet_tx_t handle_adc(packet_rx_t packet_rx) {
    packet_tx_t packet_tx = {.id = packet_rx.id, .status = STATUS_OK, .cmd = packet_rx.cmd};
    uint16_t adc;
    switch (packet_rx.cmd) {
        case CMD_ADC_DMA:
            switch (packet_rx.args[0]) {
                case 0:
                    adc_dma_stop();
                    break;
                case 1:
                    if (!adc_dma_initialized) {
                        if (!adc_dma_init()) {
                            packet_tx.status = STATUS_ERR;
                            packet_tx.data[0] = STATUS_ERR_CMD_UNKNOWN;
                            return packet_tx;
                        }
                        adc_dma_initialized = true;
                    }
                    adc_dma_start();
                    break;
            }
            adc = adc_dma_running;
            break;
        case CMD_ADC_READ_CH0:
            adc = ch_average[ADC_CH_0];
            break;
        case CMD_ADC_READ_CH1:
            adc = ch_average[ADC_CH_1];
            break;
        case CMD_ADC_READ_CH2:
            adc = ch_average[ADC_CH_2];
            break;
        case CMD_ADC_READ_CH4:
            adc = ch_average[ADC_CH_4];
            break;
        default:
            packet_tx.status = STATUS_ERR;
            packet_tx.data[0] = STATUS_ERR_CMD_UNKNOWN;
            return packet_tx;
    }
    packet_tx.data[0] = adc & 0xFF;
    packet_tx.data[1] = (adc >> 8) & 0xFF;
    return packet_tx;
}

#ifdef __cplusplus
}
#endif