package main

import (
	"slices"
	"testing"
)

const (
	ADC_DMA_OFF uint8 = iota
	ADC_DMA_ON
	ADC_DMA_STATE = 255

	CMD_ADC_DMA = 41
)

func Test_adc_dma_on(t *testing.T) {
	req := [REQ_SIZE]uint8{ID, CMD_ADC_DMA, ADC_DMA_ON}
	res := SendCommand(t, req[:], true)[0]
	exp := [RES_SIZE]uint8{StatusOk, CMD_ADC_DMA, ADC_DMA_ON}
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}

	exp = [RES_SIZE]uint8{StatusOk, req[1], req[2]}
	req = [REQ_SIZE]uint8{ID, CMD_ADC_DMA, ADC_DMA_STATE}
	res = SendCommand(t, req[:], true)[0]
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}
}

func Test_adc_dma_off(t *testing.T) {
	req := [REQ_SIZE]uint8{ID, CMD_ADC_DMA, ADC_DMA_OFF}
	res := SendCommand(t, req[:], true)[0]
	exp := [RES_SIZE]uint8{StatusOk, CMD_ADC_DMA, ADC_DMA_OFF}
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}

	exp = [RES_SIZE]uint8{StatusOk, req[1], req[2]}
	req = [REQ_SIZE]uint8{ID, CMD_ADC_DMA, ADC_DMA_STATE}
	res = SendCommand(t, req[:], true)[0]
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}
}
