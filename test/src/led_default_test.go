package main

import (
	"encoding/binary"
	"slices"
	"testing"
)

const (
	LED_OFF uint8 = iota
	LED_ON
	LED_BLINK

	CMD_LED_DEFAULT = 21
)

func Test_led_default_off(t *testing.T) {
	req := [REQ_SIZE]uint8{ID, CMD_LED_DEFAULT, LED_OFF}
	res := SendCommand(t, req[:], true)[0]
	exp := [RES_SIZE]uint8{StatusOk, CMD_LED_DEFAULT, LED_OFF}
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}
}

func Test_led_default_on(t *testing.T) {
	req := [REQ_SIZE]uint8{ID, CMD_LED_DEFAULT, LED_ON}
	res := SendCommand(t, req[:], true)[0]
	exp := [RES_SIZE]uint8{StatusOk, CMD_LED_DEFAULT, LED_ON}
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}
}

func Test_led_default_blink_step(t *testing.T) {
	step := uint8(5)
	req := [REQ_SIZE]uint8{ID, CMD_LED_DEFAULT, LED_BLINK, step}
	res := SendCommand(t, req[:], true)[0]
	exp := [RES_SIZE]uint8{StatusOk, CMD_LED_DEFAULT, LED_BLINK}
	binary.LittleEndian.PutUint16(exp[3:], uint16(step)*100)
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}
}

func Test_led_default_state(t *testing.T) {
	val := LED_OFF
	req := [REQ_SIZE]uint8{ID, CMD_LED_DEFAULT, val}
	_ = SendCommand(t, req[:])[0]

	req = [REQ_SIZE]uint8{ID, CMD_LED_DEFAULT, 255}
	res := SendCommand(t, req[:], true)[0]
	exp := [RES_SIZE]uint8{StatusOk, CMD_LED_DEFAULT, val}
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}
}

func Test_led_default_err_invalid_arg(t *testing.T) {
	req := [REQ_SIZE]uint8{ID, CMD_LED_DEFAULT, 3}
	res := SendCommand(t, req[:], true)[0]
	exp := [RES_SIZE]uint8{StatusError, CMD_LED_DEFAULT, 2}
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}
}
