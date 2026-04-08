package main

import (
	"slices"
	"testing"
)

const (
	CMD_TB6612FNG         = 60
	CMD_TB6612FNG_POWER   = 61
	CMD_TB6612FNG_MOTOR_1 = 62
	CMD_TB6612FNG_MOTOR_2 = 63
)

const (
	ARG_TB6612FNG_POWER_OFF   = 0
	ARG_TB6612FNG_POWER_ON    = 1
	ARG_TB6612FNG_POWER_STATE = 255
)

const (
	DIRECTION_STOP uint8 = iota
	DIRECTION_FORWARD
	DIRECTION_REVERSE
	DIRECTION_BRAKE
)

func req(id uint8) []uint8 {
	return []uint8{
		id, CMD_TB6612FNG_POWER, ARG_TB6612FNG_POWER_ON, 0, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 0, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 10, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 20, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 30, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 40, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 50, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 60, 0, 0, 0, 0,

		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 70, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 80, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 90, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 100, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 90, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 80, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 70, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 60, 0, 0, 0, 0,

		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 50, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 40, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 30, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 20, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 10, 0, 0, 0, 0,
		id, CMD_TB6612FNG_MOTOR_1, DIRECTION_FORWARD, 0, 0, 0, 0, 0,
		id, CMD_TB6612FNG_POWER, ARG_TB6612FNG_POWER_OFF, 0, 0, 0, 0, 0,
	}
}

func Test_get_power_on(t *testing.T) {
	req := [REQ_SIZE]uint8{ID, CMD_TB6612FNG_POWER, ARG_TB6612FNG_POWER_ON}
	res := SendCommand(t, req[:])[0]
	exp := [RES_SIZE]uint8{StatusOk, req[1], ARG_TB6612FNG_POWER_ON}
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}

	exp = [RES_SIZE]uint8{StatusOk, req[1], req[2]}
	req = [REQ_SIZE]uint8{ID, CMD_TB6612FNG_POWER, ARG_TB6612FNG_POWER_STATE}
	res = SendCommand(t, req[:])[0]
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}
}

func Test_get_power_off(t *testing.T) {
	req := [REQ_SIZE]uint8{ID, CMD_TB6612FNG_POWER, ARG_TB6612FNG_POWER_OFF}
	res := SendCommand(t, req[:])[0]
	exp := [RES_SIZE]uint8{StatusOk, req[1], ARG_TB6612FNG_POWER_OFF}
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}

	exp = [RES_SIZE]uint8{StatusOk, req[1], req[2]}
	req = [REQ_SIZE]uint8{ID, CMD_TB6612FNG_POWER, ARG_TB6612FNG_POWER_STATE}
	res = SendCommand(t, req[:])[0]
	if !slices.Equal(res, exp[:]) {
		t.Errorf("expected %v, got %v", exp, res)
	}

}

func Test_per_command(t *testing.T) {
	const runs = 1
	req := req(ID)
	reqLen := len(req)
	for range runs {
		for i := 0; i < reqLen; i += REQ_SIZE {
			Test_get_power_on(t)
			re := req[i : i+REQ_SIZE]
			res := SendCommand(t, re, true)[0]
			exp := [RES_SIZE]uint8{StatusOk, re[1], re[2], re[3]}
			if !slices.Equal(res, exp[:]) {
				t.Errorf("expected %v, got %v", exp, res)
			}

			if i > 0 && i < len(req)-1 {
				r := [REQ_SIZE]uint8{ID, 64, CMD_TB6612FNG_MOTOR_1}
				res := SendCommand(t, r[:])[0]
				exp := [RES_SIZE]uint8{StatusOk, CMD_TB6612FNG_MOTOR_1, re[2], re[3]}
				if !slices.Equal(res, exp[:]) {
					t.Errorf("expected %v, got %v", exp, res)
				}
			}
		}
		Test_get_power_off(t)
	}
}

func Test_chunk(t *testing.T) {
	const runs = 1
	req := req(ID)
	reqChunks := Chunk64(req)
	for range runs {
		var res [][]uint8
		for _, reqChunk := range reqChunks {
			res = append(res, SendCommand(t, reqChunk, true)...)
		}

		for i, r := range res {
			start := i * REQ_SIZE
			end := start + REQ_SIZE
			p := req[start:end]
			cmd := p[1]

			lenReq := len(req)

			switch {
			case i == 0:
				exp := [RES_SIZE]uint8{StatusOk, cmd, ARG_TB6612FNG_POWER_ON}
				if !slices.Equal(r, exp[:]) {
					t.Errorf("expected %v, got %v", exp, r)
				}
			case i > 0 && i < ((lenReq/REQ_SIZE)-1):
				direction := p[2]
				speed := p[3]
				exp := [RES_SIZE]uint8{StatusOk, cmd, direction, speed}
				if !slices.Equal(r, exp[:]) {
					t.Errorf("expected %v, got %v", exp, r)
				}
			case i == ((lenReq / REQ_SIZE) - 1):
				exp := [RES_SIZE]uint8{StatusOk, cmd, ARG_TB6612FNG_POWER_OFF}
				if !slices.Equal(r, exp[:]) {
					t.Errorf("expected %v, got %v", exp, r)
				}
			}

		}
	}
}

func Test_bulk(t *testing.T) {
	req := req(ID)
	res := SendCommand(t, req, true)
	reqCount := len(req) / REQ_SIZE
	resLen := len(res)
	if resLen != reqCount {
		t.Errorf("expected %v, got %v", reqCount, resLen)
	}
	for i, r := range res {
		start := i * REQ_SIZE
		end := start + REQ_SIZE
		p := req[start:end]
		cmd := p[1]

		lenReq := len(req)

		switch {
		case i == 0:
			exp := [RES_SIZE]uint8{StatusOk, cmd, ARG_TB6612FNG_POWER_ON}
			if !slices.Equal(r, exp[:]) {
				t.Errorf("expected %v, got %v", exp, r)
			}
		case i > 0 && i < ((lenReq/REQ_SIZE)-1):
			direction := p[2]
			speed := p[3]
			exp := [RES_SIZE]uint8{StatusOk, cmd, direction, speed}
			if !slices.Equal(r, exp[:]) {
				t.Errorf("expected %v, got %v", exp, r)
			}
		case i == ((lenReq / REQ_SIZE) - 1):
			exp := [RES_SIZE]uint8{StatusOk, cmd, ARG_TB6612FNG_POWER_OFF}
			if !slices.Equal(r, exp[:]) {
				t.Errorf("expected %v, got %v", exp, r)
			}
		}
	}
}
