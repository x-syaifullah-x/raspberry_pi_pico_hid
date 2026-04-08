# Pi Pico HID Command Protocol

A USB HID command protocol for the Raspberry Pi Pico (RP2040), enabling control of GPIO, ADC, PWM, and motor drivers over a HID raw device interface.

---

## Data Structure

```c
typedef struct __attribute__((packed)) {
    uint64_t sig;  // Request echo — params reflected little-endian (max 8 bytes)
    uint32_t res;  // Response payload — little-endian
} pi_pico_data;
```

### `res` Layout (uint32, little-endian)
| `byte[0]` | `byte[1]` | `byte[2]` | `byte[3]` |
|-----------|-----------|-----------|-----------|
| `0x01` on error, `0x00` on success | payload or error code | payload | payload |

---

## Error Codes

| Constant | Value | Description |
|----------|-------|-------------|
| `ERR_OK` | `0x00` | Success |
| `ERR_METHODE_NUMBER` | `0x01` | Unknown method |
| `ERR_METHOD_PARAM_INVALID` | `0x02` | Invalid parameter value |
| `ERR_METHOD_PARAM_SIZE` | `0x03` | Wrong number of parameters (max 8) |
| `ERR_METHOD_NOT_IMPLEMENTED` | `0x04` | Method recognized but not yet implemented |

---

## Methods

### `0x02` — METHODE_DEFAULT_LED

Controls the onboard LED.

```
Request:  [ID][0x02][ACTION][DELAY?]
```

| `params[2]` | Name | `params[3]` | Description |
|-------------|------|-------------|-------------|
| `0x00` | `LED_OFF` | — | Turn LED off |
| `0x01` | `LED_ON` | — | Turn LED on |
| `0x02` | `LED_BLINK` | `1–255` × 100ms (default: `1000ms`) | Blink LED at specified interval |
| `0xFF` | `LED_GET_STATUS` | — | Read current LED state |

#### Response

**OFF / ON:**
```
byte[0] = 0x00
byte[1] = gpio_get  →  0x00 (off) or 0x01 (on)
```

**BLINK:**
```
byte[0] = 0x00
byte[1] = gpio_get
byte[2] = delay_ms low byte
byte[3] = delay_ms high byte
```

**GET_STATUS — currently blinking:**
```
byte[0] = 0x00
byte[1] = 0x02  (BLINK)
byte[2] = delay_ms low byte
byte[3] = delay_ms high byte
```

**GET_STATUS — not blinking:**
```
byte[0] = 0x00
byte[1] = gpio_get  →  0x00 or 0x01
```

**Error:**
```
byte[0] = 0x01  (ERR_FLAG)
byte[1] = error code
```

#### Examples

```bash
# Turn LED on
printf '\x01\x02\x01' > /dev/hidrawN

# Blink LED every 500ms  (params[3]=5, 5×100=500ms)
printf '\x01\x02\x02\x05' > /dev/hidrawN

# Blink LED every 1000ms (default)
printf '\x01\x02\x02' > /dev/hidrawN

# Get LED status
printf '\x01\x02\xFF' > /dev/hidrawN
```

---

### `0x03` — METHODE_ADC_READ

Reads a value from an ADC channel.

```
Request:  [ID][0x03][CHANNEL]
```

| `params[2]` | Description |
|-------------|-------------|
| `0x04` | Onboard temperature sensor |

#### Response

```
byte[0] = 0x00
byte[1] = adc_read low byte   (range 0–4095)
byte[2] = adc_read high byte
```

> Optional conversion from raw ADC value:
> ```c
> float voltage = (adc_read * 3.3f) / 4095.0f;
> float temp    = 27.0f - (voltage - 0.706f) / 0.001721f;  // degrees Celsius
> ```

#### Example

```bash
# Read onboard temperature sensor
printf '\x01\x03\x04' > /dev/hidrawN
```

---

### `0x04` — METHODE_TB6612FNG

Controls the TB6612FNG dual motor driver.

```
Request:  [ID][0x04][SUB_METHOD][...]
```

---

#### `params[2] = 0x00` — POWER

Controls the STBY (standby) pin of the TB6612FNG.

```
Request:  [ID][0x04][0x00][ACTION]
```

| `params[3]` | Description |
|-------------|-------------|
| `0x00` | Power off (enter standby) |
| `0x01` | Power on (exit standby) |

**Response:**
```
byte[0] = 0x00
byte[1] = 0x00 (off) or 0x01 (on)
```

```bash
# Power on
printf '\x01\x04\x00\x01' > /dev/hidrawN

# Power off
printf '\x01\x04\x00\x00' > /dev/hidrawN
```

---

#### `params[2] = 0x01` — MOTOR

Controls motor direction and speed.

```
Request:  [ID][0x04][0x01][MOTOR][ACTION][SPEED]
```

| `params[3]` | Motor |
|-------------|-------|
| `0x01` | Motor 1 |
| `0x02` | Motor 2 *(not implemented)* |

| `params[4]` | Action |
|-------------|--------|
| `0x00` | Free (coast) |
| `0x01` | Forward |
| `0x02` | Reverse |
| `0x03` | Brake |

| `params[5]` | Description |
|-------------|-------------|
| `0–100` | Speed in percent — required for `FORWARD` and `REVERSE` only |

**Response on success:**
```
byte[0] = 0x00
```

**Response on error:**
```
byte[0] = 0x01  (ERR_FLAG)
byte[1] = error code
```

```bash
# Motor 1 forward at 100% speed
printf '\x01\x04\x01\x01\x01\x64' > /dev/hidrawN

# Motor 1 reverse at 50% speed
printf '\x01\x04\x01\x01\x02\x32' > /dev/hidrawN

# Motor 1 brake
printf '\x01\x04\x01\x01\x03' > /dev/hidrawN

# Motor 1 free (coast)
printf '\x01\x04\x01\x01\x00' > /dev/hidrawN
```

---

#### `params[2] = 0xFF` — STATUS

Reads the current driver state.

```
Request:  [ID][0x04][0xFF][TYPE][MOTOR?]
```

| `params[3]` | Type |
|-------------|------|
| `0x01` | Power status |
| `0x02` | Motor status |

**Power status response:**
```
byte[0] = 0x00
byte[1] = 0x00 (standby) or 0x01 (active)
```

**Motor status response** (`params[4]` = `0x01` or `0x02`):
```
byte[0] = 0x00
byte[1] = direction  →  0x00 FREE | 0x01 FORWARD | 0x02 REVERSE | 0x03 BRAKE
byte[2] = speed      →  0–100 (percent)
```

```bash
# Read power status
printf '\x01\x04\xFF\x01' > /dev/hidrawN

# Read Motor 1 status
printf '\x01\x04\xFF\x02\x01' > /dev/hidrawN

# Read Motor 2 status
printf '\x01\x04\xFF\x02\x02' > /dev/hidrawN
```

---

### `0xFF` — METHODE_SYSTEM

System-level operations. No response is returned — the device resets immediately.

```
Request:  [ID][0xFF][ACTION]
```

| `params[2]` | Description |
|-------------|-------------|
| `0x00` | Reboot the device |
| `0x01` | Reboot into BOOTSEL (USB mass storage) mode |

```bash
# Reboot
printf '\x01\xFF\x00' > /dev/hidrawN

# Enter BOOTSEL mode
printf '\x01\xFF\x01' > /dev/hidrawN
```

---

## Request Format Summary

| Byte | Field | Description |
|------|-------|-------------|
| `params[0]` | `ID` | Request identifier, echoed back in `sig` |
| `params[1]` | `METHOD` | Method number (see Methods above) |
| `params[2]` | `ACTION` | Sub-method or action code |
| `params[3+]` | `PARAMS` | Additional parameters (method-dependent) |

> Maximum request size: **8 bytes**. Requests exceeding 8 bytes return `ERR_METHOD_PARAM_SIZE`.

---

## Notes

- All multi-byte values use **little-endian** byte order.
- `sig` echoes the raw request bytes for correlating requests with responses.
- `ERR_FLAG (0x01)` in `byte[0]` of `res` always indicates an error; `byte[1]` contains the specific error code.
- TB6612FNG Motor 2 control (`METHODE_TB6612FNG_MOTOR_2`) is reserved and returns `ERR_METHOD_NOT_IMPLEMENTED`.