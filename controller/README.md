# Raspberry Pi Pico

Firmware for Raspberry Pi Pico used by RaspberryPiPicoCommander.

---

# Hardware Wiring

## 🔌 TB6612FNG Motor Driver Wiring

```text
Pi Pico               TB6612FNG
------------          -----------------
PIN_04 (GP2)  <---->  STBY
PIN_05 (GP3)  <---->  PWMA
PIN_06 (GP4)  <---->  AIN2  ----->  AO2  <---->  MOTOR A (-)
PIN_07 (GP5)  <---->  AIN1  ----->  AO1  <---->  MOTOR A (+)
PIN_08 (GND)  <---->  GND
PIN_09 (GP6)  <---->  BIN1  ----->  BO1  <---->  MOTOR B (+)
PIN_10 (GP7)  <---->  BIN2  ----->  BO2  <---->  MOTOR B (-)
PIN_11 (GP8)  <---->  PWMB
PIN_36 (3V3)  <---->  VCC
                      VM    <----------------->  POWER SUPPLY (+)
                      GND   <----------------->  POWER SUPPLY (-)
```
---

# Packet Layout

## Request Packet Format

```sh
printf '\x[ID]\x[CMD]\x[CMD_ARG0]\x[CMD_ARG1]\x[CMD_ARG2]\x[CMD_ARG3]\x[CMD_ARG4]\x[CMD_ARG5]' > /dev/ttyACM0
```

## Packet RX (Request)

| Byte | Field      | Value       | Description      |
|------|------------|-------------|------------------|
| 
| `1`  | `CMD`      | `<0..255>` | Command          |
| `2`  | `CMD_ARG0` | `<0..255>` | Command argument |
| `3`  | `CMD_ARG1` | `<0..255>` | Command argument |
| `4`  | `CMD_ARG2` | `<0..255>` | Command argument |
| `5`  | `CMD_ARG3` | `<0..255>` | Command argument |
| `6`  | `CMD_ARG4` | `<0..255>` | Command argument |
| `7`  | `CMD_ARG5` | `<0..255>` | Command argument |


## Packet TX (Response)

| Byte | Field    | Value      | Description   |
|------|----------|------------|---------------|
| `0`  | `ID`     | `<0..255>` | Packet ID     |
| `1`  | `STATUS` | `0`        | Status OK     |
|      |          | `1`        | Status ERR    |
| `2`  | `CMD`    | `<0..255>` | Command       |
| `3`  | `DATA_0` | `<0..255>` | Response data |
| `4`  | `DATA_1` | `<0..255>` | Response data |
| `5`  | `DATA_2` | `<0..255>` | Response data |
| `6`  | `DATA_3` | `<0..255>` | Response data |
| `7`  | `DATA_4` | `<0..255>` | Response data |

| CODE_ERR | DESCRIPTION                     |
|:--------:|---------------------------------|
| `0`      | STATUS_ERR_CMD_UNKNOWN          |
| `1`      | STATUS_ERR_CMD_NOT_IMPLEMENTED  |
| `2`      | STATUS_ERR_CMD_ARG_INVALID      |
| `3`      | STATUS_ERR_CMD_ARG_SIZE_INVALID |
---

# 🔄 Boot ROM Control

## Reboot System

### Packet RX (Request)

| Byte   | Field      | Value      | Description                             |
|--------|------------|:----------:|-----------------------------------------|
| `0`    | `ID`       | `<1..255>` | Packet ID                               |
| `1`    | `CMD`      | `1`        | Boot ROM command                        |
| `2`    | `CMD_ARG0` | `0`        | Watcdog reboot                          |

## Reset USB

### Packet RX (Request)

| Byte   | Field      | Value      | Description                            |
|--------|------------|:----------:|----------------------------------------|
| `0`    | `ID`       | `<1..255>` | Packet ID                              |
| `1`    | `CMD`      | `1`        | Boot ROM command                       |
| `2`    | `CMD_ARG0` | `1`        | Reset USB boot                         |
| `3`    | `CMD_ARG1` | `0`        | Enable both interfaces                 |
|        |            | `1`        | Disable the USB Mass Storage Interface |
|        |            | `2`        | Disable the USB PICOBOOT Interface     |
---

# 💡 Default LED Control

## Set LED State

### Packet RX (Request)

| Byte   | Field       | Value      | Description    |
|--------|-------------|:----------:|----------------|
| `0`    | `ID`        | `<1..255>` | Packet ID       |
| `1`    | `CMD`       | `21`       | LED command    |
| `2`    | `CMD_ARG0`  | `0`        | Mode OFF        |
|        |             | `1`        | Mode ON         |
|        |             | `2`        | Mode Blink      |
|        |             | `255`      | Get LED State |
| `3`    | `CMD_ARG1`  | `<0..255>` | Blink delay     |

### Packet TX (Response OK)

| ID          | STATUS    | CMD       | MODE           | DELAY_MS       |
|:-----------:|:---------:|:---------:|:--------------:|:--------------:|
| `byte[0]`   | `byte[1]` | `byte[2]` | `byte[3]`      | `byte[4..5]`   |
| `<0..255>`  | `0`       | `21`      | `<0\|1\|2>`    | `<0..65535>`   |

| MODE | DESCRIPTION |
|------|-------------|
| `0` | LED OFF      |
| `1` | LED ON       |
| `2` | LED BLINK    |

### Packet TX (Response ERR)

| ID         | STATUS    | CMD       | CODE_ERR       |
|:----------:|:---------:|:---------:|:---------------|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]`      |
| `<0..255>` | `1`       | `21`      | `<0..255>`     |
 
---

# 📊 ADC

## ADC DMA

### Packet RX (Request)

| Byte   | Field       | Value      | Description     |
|--------|-------------|:----------:|-----------------|
| `0`    | `ID`        | `<1..255>` | Packet ID       |
| `1`    | `CMD`       | `41`       | ADC DMA command |
| `2`    | `CMD_ARG0`  | `0`        | ADC DMA Stop    |
|        |             | `1`        | ADC DMA Start   |
|        |             | `255`        | ADC DMA State   |

### Packet TX (Response OK)

| ID         | STATUS    | CMD       | STATE     | RES       |
|:----------:|:---------:|:---------:|:---------:|:---------:|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]` | `byte[4]` |
| `<0..255>` | `0`       | `41`      | `<0\|1>`  | `<0\|1>`  |

| STATE | DESCRIPTION |
|:-----:|-------------|
| `0`   | OFF         |
| `1`   | ON          |

| RES   | DESCRIPTION |
|:-----:|-------------|
| `0`   | OFF         |
| `1`   | ON          |

### Packet TX (Response ERR)

| ID         | STATUS    | CMD       | CODE_ERR       |
|:----------:|:---------:|:---------:|:---------------|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]`      |
| `<0..255>` | `1`       | `41`      | `<0..255>` |
---

## ADC Read CH0

### Packet RX (Request)

| Byte   | Field       | Value      | Description           |
|--------|-------------|:----------:|-----------------------|
| `0`    | `ID`        | `<1..255>` | Packet ID             |
| `1`    | `CMD`       | `42`       | Read ADC CH0 (GPIO26) |

### Packet TX (Response OK)

| ID         | STATUS    | CMD       | RAW          |
|:----------:|:---------:|:---------:|:------------:|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3..4]` |
| `<0..255>` | `0`       | `42`      | `<0..65535>` |

### Packet TX (Response ERR)

| ID         | STATUS    | CMD       | CODE_ERR   |
|:----------:|:---------:|:---------:|:-----------|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]`  |
| `<0..255>` | `1`       | `42`      | `<0..255>` |
---

## ADC Read CH1

### Packet RX (Request)

| Byte   | Field       | Value      | Description           |
|--------|-------------|:----------:|-----------------------|
| `0`    | `ID`        | `<1..255>` | Packet ID             |
| `1`    | `CMD`       | `43`       | Read ADC CH1 (GPIO27) |

### Packet TX (Response OK)

| ID         | STATUS    | CMD       | RAW          |
|:----------:|:---------:|:---------:|:------------:|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3..4]` |
| `<0..255>` | `0`       | `43`      | `<0..65535>` |

### Packet TX (Response ERR)

| ID         | STATUS    | CMD       | CODE_ERR   |
|:----------:|:---------:|:---------:|:-----------|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]`  |
| `<0..255>` | `1`       | `43`      | `<0..255>` |
---

## ADC Read CH2

### Packet RX (Request)

| Byte   | Field       | Value      | Description           |
|--------|-------------|:----------:|-----------------------|
| `0`    | `ID`        | `<1..255>` | Packet ID             |
| `1`    | `CMD`       | `44`       | Read ADC CH2 (GPIO28) |

### Packet TX (Response OK)

| ID         | STATUS    | CMD       | RAW          |
|:----------:|:---------:|:---------:|:------------:|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3..4]` |
| `<0..255>` | `0`       | `44`      | `<0..65535>` |

### Packet TX (Response ERR)

| ID         | STATUS    | CMD       | CODE_ERR   |
|:----------:|:---------:|:---------:|:-----------|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]`  |
| `<0..255>` | `1`       | `44`      | `<0..255>` |
---

## ADC Read CH4

### Packet RX (Request)

| Byte   | Field       | Value       | Description                         |
|--------|-------------|:-----------:|-------------------------------------|
| `0`    | `ID`        | `<1..255>` | Packet ID                            |
| `1`    | `CMD`       | `46`         | Read ADC CH4 (ONBOARD TEMP SENSOR) |

### Packet TX (Response OK)

| ID         | STATUS    | CMD       | RAW          |
|:----------:|:---------:|:---------:|:------------:|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3..4]` |
| `<0..255>` | `0`       | `46`      | `<0..65535>` |

### Packet TX (Response ERR)

| ID         | STATUS    | CMD       | CODE_ERR   |
|:----------:|:---------:|:---------:|:-----------|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]`  |
| `<0..255>` | `1`       | `46`      | `<0..255>` |
---

# ⚙️ Motor Control (TB6612FNG)

## Driver Power Control

### Packet RX (Request)

| Byte | Field      | Value      | Description        |
|------|------------|:----------:|--------------------|
| `0`  | `ID`       | `<1..255>` | Packet ID          |
| `1`  | `CMD`      | `61`       | Motor command      |
| `2`  | `CMD_ARG0` | `0`        | Driver power OFF   |
|      |            | `1`        | Driver power ON    |
|      |            | `255`        | Driver power STATE |

### Packet TX (Response OK)

| ID         | STATUS    | CMD       | POWER    |
|:----------:|:---------:|:---------:|:---------|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]`|
| `<0..255>` | `0`       | `61`      | `<0\|1>` |

| POWER | DESCRIPTION |
|:-----:|-------------|
| `0`   | OFF         |
| `1`   | ON          |

### Packet TX (Response ERR)

| ID         | STATUS    | CMD       | CODE_ERR   |
|:----------:|:---------:|:---------:|:-----------|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]`  |
| `<0..255>` | `1`       | `61`      | `<0..255>` |

## Motor Control

### Packet RX (Request)

| Byte   | Field      | Value      | Description       |
|--------|------------|:----------:|-------------------|
| `0`    | `ID`       | `<1..255>` | Packet ID         |
| `1`    | `CMD`      | `62`       | Motor 1 command   |
|        |            | `63`       | Motor 2 command   |
| `2`    | `CMD_ARG1` | `0`        | Direction Free    |
|        |            | `1`        | Direction Forward |
|        |            | `2`        | Direction Reverse |
|        |            | `3`        | Direction Brake   |
| `3`    | `CMD_ARG2` | `<0..100>` | Speed             |

### Packet TX (Response Ok)

| ID         | STATUS    | CMD        | DIRECTION      | SPEED      |
|:----------:|:---------:|:----------:|:--------------:|:----------:|
| `byte[0]`  | `byte[1]` | `byte[2]`  | `byte[3]`      | `byte[4]`  |
| `<0..255>` | `0`       | `<62\|63>` | `<0\|1\|2\|3>` | `<0..100>` |

| DIRECTION | DESCRIPTION |
|:---------:|:------------|
| `0`       | Free        |
| `1`       | Forward     |
| `2`       | Reverse     |
| `3`       | Brake       |

### Packet TX (Response ERR)

| ID         | STATUS    | CMD        | CODE_ERR   |
|:----------:|:---------:|:----------:|:-----------|
| `byte[0]`  | `byte[1]` | `byte[2]`  | `byte[3]`  |
| `<0..255>` | `1`       | `<62\|63>` | `<0..255>` |

## Get Motor State

### Packet RX (Request)

| Byte | Field      | Value      | Description   |
|------|------------|:----------:|---------------|
| `0`  | `ID`       | `<1..255>` | Packet ID     |
| `1`  | `CMD`      | `64`       | Motor command |
| `2`  | `CMD_ARG0` | `62`       | State Motor 1 |
|      |            | `63`       | State Motor 2 |


### Packet TX (Response)

| ID         | STATUS    | CMD        | MOTOR_DIRECTION | MOTOR_SPEED |
|:----------:|:---------:|:----------:|:---------------:|:-----------:|
| `byte[0]`  | `byte[1]` | `byte[2]`  | `byte[3]`       | `byte[4]`   |
| `<0..255>` | `0`       | `<62\|63>` | `<0\|1\|2\|3>`  | `<0..100>`  |

| CMD  | DESCRIPTION |
|------|-------------|
| `62` | Motor 1     |
| `63` | Motor 2     |


| MOTOR_DIRECTION | DESCRIPTION |
|----------------|----------------|
| `0`            | Free           |
| `1`            | Forward        |
| `2`            | Reverse        |
| `3`            | Brake          |

### Packet TX (Response ERR)

| ID         | STATUS    | CMD       | CODE_ERR   |
|:----------:|:---------:|:---------:|:-----------|
| `byte[0]`  | `byte[1]` | `byte[2]` | `byte[3]`  |
| `<0..255>` | `1`       | `64`      | `<0..255>` |

---