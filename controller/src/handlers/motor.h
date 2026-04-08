#pragma once

#include "drivers/motor/driver_motor_bts7960.h"
#include "drivers/motor/driver_motor_drv8833.h"
#include "drivers/motor/driver_motor_tb6612fng.h"
#include "drivers/motor/driver_motor_zk5ad.h"
#include "motor.h"
#include "packet/rx/packet_rx.h"
#include "packet/tx/packet_tx.h"

#ifdef __cplusplus
extern "C" {
#endif

#define MOTOR_MAX_COUNT ((CMD_MOTOR_END) - (CMD_MOTOR))

typedef enum {
    MOTOR_DRIVER_BTS7960,
    MOTOR_DRIVER_DRV8833,
    MOTOR_DRIVER_TB6612FNG,
    MOTOR_DRIVER_ZK5AD,
} motor_driver_type_t;

typedef enum {
    MOTOR_CMD_INIT,
    MOTOR_CMD_DEINIT,
    MOTOR_CMD_SET_DIRECTION,
    MOTOR_CMD_SET_SPEED,
    MOTOR_CMD_STATE = 0xFF,
} motor_cmd_t;

typedef struct {
    bool active;
    motor_driver_t driver;
    motor_driver_type_t type;
    union {
        bts7960_t bts7960;
        drv8833_t drv8833;
        tb6612fng_t tb6612fng;
        zk5ad_t zk5ad;
    } ctx;
} motor_slot_t;

static inline motor_result_t motor_create_driver(motor_slot_t* slot, motor_driver_type_t type, packet_rx_t rx) {
    switch (type) {
        case MOTOR_DRIVER_BTS7960:
            slot->ctx.bts7960 = (bts7960_t){
                .r_en = rx.args[2],
                .rpwm = {.gpio = rx.args[3]},
                .l_en = rx.args[4],
                .lpwm = {.gpio = rx.args[5]},
            };
            return driver_bts7960_create(&slot->driver, &slot->ctx.bts7960);

        case MOTOR_DRIVER_DRV8833:
            slot->ctx.drv8833 = (drv8833_t){
                .stby = rx.args[2],
                .in1 = {.gpio = rx.args[3]},
                .in2 = {.gpio = rx.args[4]},
            };
            return driver_drv8833_create(&slot->driver, &slot->ctx.drv8833);

        case MOTOR_DRIVER_TB6612FNG:
            slot->ctx.tb6612fng = (tb6612fng_t){
                .stby = rx.args[2],
                .in1 = rx.args[3],
                .in2 = rx.args[4],
                .pwm = {.gpio = rx.args[5]},
            };
            return driver_tb6612fng_create(&slot->driver, &slot->ctx.tb6612fng);

        case MOTOR_DRIVER_ZK5AD:
            slot->ctx.zk5ad = (zk5ad_t){
                .pwm_a.gpio = rx.args[2],
                .pwm_b.gpio = rx.args[3],
            };
            return driver_zk5ad_create(&slot->driver, &slot->ctx.zk5ad);

        default:
            return MOTOR_RESULT_ERR;
    }
}

// return: [ID, STATUS, CMD, MOTOR_ID, MOTOR_ACTIVE, MOTOR_DIRECTION, MOTOR_SPEED, 0]
static inline packet_tx_t handle_motor(packet_rx_t rx) {
    packet_tx_t tx = {.id = rx.id, .status = STATUS_OK, .cmd = rx.cmd};

    static motor_slot_t slots[MOTOR_MAX_COUNT] = {0};

    // INIT: su -pc "printf '\x00\x3d\x00\x00\xff\x0f\xff\x0e' > /dev/ttyACM0"
    // DIRECTION: su -pc "printf '\x00\x3d\x02\x01\x01\x0f\xff\x0e' > /dev/ttyACM0"
    // SPEED: su -pc "printf '\x00\x3d\x03\x01\x0a\x00\x00\x00' > /dev/ttyACM0"
    switch ((motor_cmd_t)rx.args[0]) {
        case MOTOR_CMD_INIT: {
            int free_index = -1;
            for (uint32_t i = 0; i < MOTOR_MAX_COUNT; i++) {
                if (!slots[i].active) {
                    free_index = i;
                    break;
                }
            }
            if (free_index < 0) {
                tx.status = STATUS_ERR;
                return tx;
            }

            motor_slot_t* slot = &slots[free_index];
            motor_driver_type_t type = (motor_driver_type_t)rx.args[1];

            if (motor_create_driver(slot, type, rx) != MOTOR_RESULT_OK) {
                tx.status = STATUS_ERR;
                return tx;
            }

            slot->driver.ops->init(&slot->driver);
            slot->type = type;
            slot->active = true;

            const uint8_t motor_id = free_index + 1;

            tx.cmd = tx.cmd + motor_id;
            tx.data[0] = motor_id;
            tx.data[1] = slot->active;
            tx.data[2] = slot->driver.state.direction;
            tx.data[3] = slot->driver.state.speed;
            return tx;
        }

        case MOTOR_CMD_DEINIT: {
            const uint8_t motor_id = rx.args[1];
            motor_slot_t* slot = &slots[motor_id - 1];

            if (motor_id >= MOTOR_MAX_COUNT || !slot->active) {
                tx.status = STATUS_ERR;
                return tx;
            }

            slot->driver.ops->deinit(&slot->driver);
            slot->active = false;

            tx.cmd = tx.cmd + motor_id;
            tx.data[0] = motor_id;
            tx.data[1] = slot->active;
            tx.data[2] = slot->driver.state.direction;
            tx.data[3] = slot->driver.state.speed;
            return tx;
        }

        case MOTOR_CMD_SET_DIRECTION: {
            const uint8_t motor_id = rx.args[1];
            motor_slot_t* slot = &slots[motor_id - 1];
            if (motor_id >= MOTOR_MAX_COUNT || !slot->active) {
                tx.status = STATUS_ERR;
                return tx;
            }

            const motor_direction_t direction = rx.args[2];
            slot->driver.ops->set_direction(&slot->driver, direction);

            tx.cmd = tx.cmd + motor_id;
            tx.data[0] = motor_id;
            tx.data[1] = slot->active;
            tx.data[2] = slot->driver.state.direction;
            tx.data[3] = slot->driver.state.speed;
            return tx;
        }

        case MOTOR_CMD_SET_SPEED: {
            const uint8_t motor_id = rx.args[1];
            motor_slot_t* slot = &slots[motor_id - 1];

            if (motor_id >= MOTOR_MAX_COUNT || !slot->active) {
                tx.status = STATUS_ERR;
                return tx;
            }

            uint8_t speed = rx.args[2];
            slot->driver.ops->set_speed(&slot->driver, speed);

            tx.cmd = tx.cmd + motor_id;
            tx.data[0] = motor_id;
            tx.data[1] = slot->active;
            tx.data[2] = slot->driver.state.direction;
            tx.data[3] = slot->driver.state.speed;
            return tx;
        }

        case MOTOR_CMD_STATE: {
            const uint8_t motor_id = rx.args[1];
            motor_slot_t* slot = &slots[motor_id - 1];
            tx.cmd = tx.cmd + motor_id;
            tx.data[0] = motor_id;
            tx.data[1] = slot->active;
            tx.data[2] = slot->driver.state.direction;
            tx.data[3] = slot->driver.state.speed;
            return tx;
        }

        default: {
            tx.status = STATUS_ERR;
            return tx;
        }
    }
}

#ifdef __cplusplus
}
#endif