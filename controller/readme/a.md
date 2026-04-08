# RP2040 Pico Commander (Device Controller Firmware)

Firmware C untuk Raspberry Pi Pico (RP2040) yang mengubah board menjadi **device controller** serba guna: LED (built-in & WS2812), pembacaan ADC (dengan DMA + round-robin + sensor suhu internal), dan kontrol motor DC multi-driver (BTS7960, DRV8833, TB6612FNG, ZK5AD). Komunikasi dengan host (PC/browser/aplikasi lain) menggunakan **protokol paket biner 8-byte** yang dikirim lewat USB (Vendor/CDC/HID) dan/atau UART.

Repo asal: https://github.com/x-syaifullah-x/raspberry_pi_pico_commander

---

## Daftar Isi

1. [Fitur](#fitur)
2. [Arsitektur & Struktur Proyek](#arsitektur--struktur-proyek)
3. [Kebutuhan (Prerequisites)](#kebutuhan-prerequisites)
4. [Instalasi & Build](#instalasi--build)
5. [Flashing ke Pico](#flashing-ke-pico)
6. [Konfigurasi USB & Pin](#konfigurasi-usb--pin)
7. [Protokol Paket (Wire Protocol)](#protokol-paket-wire-protocol)
8. [Daftar Perintah (Command Reference)](#daftar-perintah-command-reference)
9. [Contoh Penggunaan](#contoh-penggunaan)
10. [Monitoring / Debug](#monitoring--debug)
11. [Troubleshooting](#troubleshooting)

---

## Fitur

- **LED default (onboard)**: ON / OFF / Blink dengan interval yang bisa diatur, serta baca status.
- **LED WS2812 (NeoPixel, 1 buah, RGB/RGBW)**: init/deinit PIO dan set warna.
- **ADC**: pembacaan 3 channel analog + sensor suhu internal, dengan mode DMA + round-robin sampling (rata-rata otomatis).
- **Kontrol Motor DC** hingga beberapa slot motor sekaligus, mendukung 4 jenis driver:
  - BTS7960
  - DRV8833
  - TB6612FNG
  - ZK5AD

  Setiap motor mendukung INIT, DEINIT, SET_DIRECTION (coast/forward/reverse/brake), SET_SPEED (0–100%), dan STATE (baca status).
- **Transport ganda**: USB (interface CDC untuk log serial + Vendor untuk data biner + HID) dan UART0 (GPIO16/GPIO17).
- **Reboot / BOOTSEL jarak jauh** lewat paket khusus (`CMD_SYSTEM_REBOOT`), tanpa perlu menekan tombol fisik.
- Deskriptor USB custom (Vendor ID `0x1902`, Product ID `0x1992`) dengan info `picotool info` yang informatif (pinout power, versi, dsb).

---

## Arsitektur & Struktur Proyek

```
.
├── CMakeLists.txt              # Build system (Pico SDK)
├── Makefile                    # Shortcut build/flash/monitor
├── src/
│   ├── app/
│   │   ├── main.c              # Entry point, main loop, IRQ handler UART
│   │   └── dispatcher.h        # Router: cmd -> handler
│   ├── config/
│   │   └── config_usb.h        # Deskriptor USB (CDC + HID + Vendor), string desc
│   ├── hal/                    # Hardware Abstraction Layer
│   │   ├── pin.h                # Mapping nama pin fisik <-> GPIO
│   │   ├── uart.h / uart.c      # Inisialisasi UART0
│   │   └── pwm.h                 # Helper PWM generik
│   ├── handlers/                # Logic per kategori command
│   │   ├── boot.h               # Reboot / masuk BOOTSEL
│   │   ├── adc.h                 # ADC + DMA + round robin + suhu
│   │   ├── motor.h               # Router command motor -> driver aktif
│   │   ├── fnr.h                  # Forward/Neutral/Reverse switch (opsional, digital in)
│   │   └── led/, ws2812/          # Handler LED default & WS2812
│   ├── drivers/motor/            # Implementasi tiap jenis driver motor
│   │   ├── driver_motor.h            # Interface umum (ops: init/deinit/set_direction/set_speed)
│   │   ├── driver_motor_bts7960.*
│   │   ├── driver_motor_drv8833.*
│   │   ├── driver_motor_tb6612fng.*
│   │   └── driver_motor_zk5ad.*
│   ├── packet/
│   │   ├── packet.h              # Enum CMD, STATUS, ID
│   │   ├── rx/                    # Struct paket masuk, ring buffer, proses FIFO/BUF
│   │   └── tx/                    # Struct paket keluar, log tx
│   └── tusb_config.h             # Konfigurasi TinyUSB (CDC/HID/Vendor)
└── build/                        # Hasil build (dibuat otomatis, di-gitignore)
```

**Alur data:**

```
Host (PC/browser/HP)
   │  paket 8-byte biner via USB (Vendor/CDC/HID) atau UART0
   ▼
IRQ handler (uart_0_irq_handler / tud_vendor_rx_cb / tud_cdc_rx_cb / tud_hid_set_report_cb)
   │  push ke ring buffer (packet_rx_rb)
   ▼
main loop -> packet_rx_rb_pop() -> dispatch(rx)
   │  dispatcher.h merutekan berdasarkan rentang CMD
   ▼
handler (led / adc / motor) -> driver terkait (jika motor)
   │  hasil dikemas jadi packet_tx_t
   ▼
Dikirim balik ke host lewat UART0 dan/atau USB Vendor dan/atau HID
```

---

## Kebutuhan (Prerequisites)

- Board **Raspberry Pi Pico** (RP2040).
- **Pico C/C++ SDK** (`pico-sdk`) — proyek ini meng-include `lib/pico-sdk/pico_sdk_init.cmake`, jadi SDK harus tersedia sebagai submodule/folder di `lib/pico-sdk` relatif terhadap root proyek (clone terpisah atau `git submodule add`).
- Toolchain ARM: `gcc-arm-none-eabi`, `libstdc++-arm-none-eabi-newlib`.
- `cmake` (>= 3.13).
- `picotool` (untuk flashing & info device).
- `python3`.

### Instal dependency (Debian/Ubuntu)

Makefile sudah menyediakan target siap pakai:

```bash
make dependencies
```

Ini setara dengan:

```bash
sudo apt install --no-install-suggests --no-install-recommends -y \
    cmake gcc-arm-none-eabi picotool libstdc++-arm-none-eabi-newlib python3
```

### Menyiapkan Pico SDK

Jika belum ada folder `lib/pico-sdk`, clone dulu (sesuaikan tag versi SDK yang stabil):

```bash
mkdir -p lib
git clone -b master --recurse-submodules https://github.com/raspberrypi/pico-sdk.git lib/pico-sdk
export PICO_SDK_PATH="$(pwd)/lib/pico-sdk"
```

---

## Instalasi & Build

1. **Clone repo** (jika belum):
   ```bash
   git clone https://github.com/x-syaifullah-x/raspberry_pi_pico_commander.git
   cd raspberry_pi_pico_commander
   ```

2. **Install dependency**:
   ```bash
   make dependencies
   ```

3. **Pastikan `lib/pico-sdk` tersedia** (lihat bagian [Kebutuhan](#kebutuhan-prerequisites) di atas).

4. **Configure (generate build system)**:
   ```bash
   make configure
   ```
   Setara dengan `cmake -S . -B build`.

5. **Build firmware**:
   ```bash
   make build
   ```
   Setara dengan `cmake --build build`. Setelah sukses, akan muncul `build/controller.uf2` (dan `.elf`, `.bin`, `.map`).

6. **Bersihkan build** (jika perlu build ulang dari nol):
   ```bash
   make clean
   ```

---

## Flashing ke Pico

Ada dua cara umum:

### A. Via `picotool` (disarankan)

Board harus dalam mode **BOOTSEL** dulu (tahan tombol BOOTSEL saat mencolokkan kabel USB), lalu:

```bash
make flash
```

Ini akan menjalankan:
```bash
picotool load build/controller.uf2
picotool info -a
```

Atau langsung flash + reboot otomatis ke firmware:
```bash
make flash_and_reboot
```

### B. Via Mass Storage (drag & drop UF2)

1. Tahan tombol **BOOTSEL** di Pico, colokkan USB ke komputer, lalu lepas tombolnya. Pico akan muncul sebagai mass storage `RPI-RP2`.
2. Salin `build/controller.uf2` ke drive tersebut:
   ```bash
   cp build/controller.uf2 /media/$USER/RPI-RP2/
   ```
3. Pico otomatis reboot dan menjalankan firmware baru.

> Target `flash` di Makefile menyediakan alternatif `cp` (baris di-comment) — bisa diaktifkan lagi jika lebih suka metode drag & drop tanpa `picotool`.

### Reboot ulang ke BOOTSEL tanpa cabut kabel (setelah firmware ini berjalan)

Jika device sudah menjalankan firmware ini dan terdeteksi sebagai `/dev/ttyACM0` (CDC), gunakan:

```bash
make bootsel TTYACM=/dev/ttyACM0
```

Perintah ini mengirim paket biner `CMD_SYSTEM_REBOOT` (`0x01`) dengan sub-perintah `RESET_USB_BOOT` ke port serial, sehingga Pico langsung masuk mode BOOTSEL tanpa perlu ditekan manual.

---

## Konfigurasi USB & Pin

### Identitas USB

| Item | Nilai |
|---|---|
| Vendor ID | `0x1902` |
| Product ID | `0x1992` |
| Manufacturer | Raspberry PI Pico |
| Product | RP2040 |
| Interface | CDC (serial log/console), HID (in/out 64-byte report), Vendor (bulk data biner) |

Diatur lewat `target_compile_definitions` di `CMakeLists.txt` — ubah `USB_ID_VENDOR`, `USB_ID_PRODUCT`, dsb. sesuai kebutuhan sebelum build.

### Pin Power

| Pin fisik | Fungsi |
|---|---|
| PIN_40 | VBUS (5V dari USB) |
| PIN_39 | VSYS (input 2–5V) |
| PIN_36 | 3V3OUT (output 3.3V) |
| PIN_03, 08, 13, 18, 23, 28, 33, 38 | GND |

### Pin Fungsional Bawaan Firmware

| Fungsi | Pin | Keterangan |
|---|---|---|
| UART0 TX | `PIN_21` (GP16) | Baudrate default = `PICO_DEFAULT_UART_BAUD_RATE` (biasanya 115200) |
| UART0 RX | `PIN_22` (GP17) | |
| WS2812 data | `PIN_21` (GP16) | **Catatan:** definisi `WS2812_PIN` sama dengan UART0 TX. Jangan menyambungkan strip WS2812 dan perangkat UART pada pin yang sama secara bersamaan — pilih salah satu, atau ubah `WS2812_PIN` di `src/handlers/ws2812/ws2812.h` sebelum build jika ingin memakai keduanya. |
| ADC channel 0–2 | `PIN_31`/GP26, `PIN_32`/GP27, `PIN_34`/GP28 | Round-robin + DMA, hasil berupa rata-rata 256 sample |
| ADC channel internal (suhu) | — | Sensor suhu built-in RP2040 |
| LED onboard | `PICO_DEFAULT_LED_PIN` | ON/OFF/Blink |
| Pin motor (R_EN/L_EN/RPWM/LPWM/STBY/IN1/IN2/PWM_A/PWM_B, dst) | **Dikirim dinamis dari host** saat `MOTOR_CMD_INIT` (lihat argumen paket di bawah) — tidak hardcode di firmware. |

Mapping nama pin fisik (`PIN_xx`) ke nomor GPIO ada di `src/hal/pin.h`.

---

## Protokol Paket (Wire Protocol)

Semua komunikasi memakai paket **biner 8 byte, ukuran tetap** (power-of-two, memudahkan alignment dan pemrosesan cepat).

### Paket masuk (Host → Device) — `packet_rx_t`

```
Byte:   0      1      2      3      4      5      6      7
      [ id ] [cmd ] [a0  ] [a1  ] [a2  ] [a3  ] [a4  ] [a5  ]
```

| Field | Ukuran | Keterangan |
|---|---|---|
| `id` | 1 byte | `0x00` = ID_DEVICE, `0x01` = ID_HOST |
| `cmd` | 1 byte | Kode perintah (lihat [Daftar Perintah](#daftar-perintah-command-reference)) |
| `args[0..5]` | 6 byte | Argumen, arti tergantung `cmd` |

### Paket keluar (Device → Host) — `packet_tx_t`

```
Byte:   0      1        2      3      4      5      6      7
      [ id ] [status] [cmd ] [d0  ] [d1  ] [d2  ] [d3  ] [d4  ]
```

| Field | Ukuran | Keterangan |
|---|---|---|
| `id` | 1 byte | Sama seperti paket masuk (di-echo balik) |
| `status` | 1 byte | `0x00` = OK, `0x01` = ERR |
| `cmd` | 1 byte | CMD echo (khusus motor, di-offset dengan `motor_id`, lihat catatan di bawah) |
| `data[0..4]` | 5 byte | Data balasan, arti tergantung `cmd` |

Jika `status == STATUS_ERR`, `data[0]` berisi kode error:

| Kode | Arti |
|---|---|
| `0x00` | `CMD_UNKNOWN` — command tidak dikenal |
| `0x01` | `CMD_NOT_IMPLEMENTED` |
| `0x02` | `CMD_ARG_INVALID` — argumen tidak valid |
| `0x03` | `CMD_ARG_SIZE_INVALID` |

### Jalur transport

- **UART0** (GPIO16 TX / GPIO17 RX) — selalu aktif (`UART_0_ENABLE 1`), paket dibaca via IRQ per-byte.
- **USB CDC** — jika stdio-USB aktif, dipakai juga sebagai jalur RX paket (`tud_cdc_rx_cb`), berguna untuk devboard yang diakses lewat `/dev/ttyACM0`. Print log debug (`printf`) hanya aktif jika `LIB_PICO_STDIO_USB` terdefinisi saat build.
- **USB Vendor (bulk)** — jalur data biner murni, direkomendasikan untuk aplikasi (mis. WebUSB/browser control panel) karena tidak tercampur dengan log teks (`tud_vendor_rx_cb` / `tud_vendor_write`).
- **USB HID** — juga tersedia sebagai jalur alternatif (report 64 byte in/out).

Semua paket yang diterima lewat jalur manapun masuk ke **ring buffer** yang sama (`packet_rx_rb`, kapasitas 16 slot), lalu diproses satu per satu di `main()` dan hasilnya dikirim balik ke **semua jalur output yang aktif** (UART + Vendor + HID) secara bersamaan.

---

## Daftar Perintah (Command Reference)

### Rentang kode command (`cmd_t`)

| Range | Kategori |
|---|---|
| `1` | `CMD_SYSTEM_REBOOT` |
| `2–19` | (reserved, system) |
| `20–29` | LED (`21` = LED default, `22` = WS2812) |
| `40–59` | ADC (`41` DMA, `42–46` baca channel 0–4) |
| `60–69` | Motor (`61` = `CMD_MOTOR`) |

### 1. System — Reboot / BOOTSEL (`CMD_SYSTEM_REBOOT = 1`)

| args[0] | Aksi |
|---|---|
| `0x00` (`REBOOT`) | Watchdog reboot biasa |
| `0x01` (`RESET_USB_BOOT`) | Masuk mode BOOTSEL. `args[1]` = pilihan LED mask (0–2), nilai lain diabaikan |

Perintah ini diproses **langsung saat menerima paket**, tidak menunggu antrean, sehingga selalu responsif.

### 2. LED Default / Onboard (`CMD_LED_DEFAULT = 21`)

| args[0] | Aksi | args[1] |
|---|---|---|
| `0x00` | LED OFF | — |
| `0x01` | LED ON | — |
| `0x02` | Blink | `delay_step` (1–255), delay aktual = `delay_step × 100ms` |
| `0xFF` | Baca status saat ini | — |

Balasan (`data`): `[mode, delay_ms_low, delay_ms_high]`.

### 3. LED WS2812 / NeoPixel (`CMD_LED_WS2812_1BIT = 22`)

| args[0] | Aksi | Argumen lain |
|---|---|---|
| `0` | Init/Deinit | `args[1]`: `1` = init (nyalakan PIO), `0` = deinit |
| `1` | Set warna | `args[1..3]` = R, G, B (dan `args[4]` = W jika `IS_RGBW`) |

Balasan (`data`) untuk set warna: `[1, R, G, B, (W)]`.

### 4. ADC (`CMD_ADC_DMA = 41`, `CMD_ADC_READ_CH0..4 = 42–46`)

| Command | args[0] | Aksi |
|---|---|---|
| `CMD_ADC_DMA` (41) | `0` | Stop DMA sampling |
| `CMD_ADC_DMA` (41) | `1` | Start/aktifkan DMA sampling (round-robin CH0–2 + suhu) |
| `CMD_ADC_READ_CH0` (42) | — | Baca rata-rata channel 0 (GP26) |
| `CMD_ADC_READ_CH1` (43) | — | Baca rata-rata channel 1 (GP27) |
| `CMD_ADC_READ_CH2` (44) | — | Baca rata-rata channel 2 (GP28) |
| `CMD_ADC_READ_CH4` (46) | — | Baca suhu internal (raw ADC) |

Balasan (`data`): `[value_low, value_high]` (uint16 little-endian, hasil rata-rata 256 sample untuk channel analog, 16 sample untuk suhu).

> Saat DMA aktif, firmware otomatis mem-push paket `READ_CHx` internal setiap 1 detik lewat timer, jadi host **tidak wajib** polling manual — tapi tetap bisa query kapan saja untuk nilai rata-rata terbaru.

### 5. Motor (`CMD_MOTOR = 61`)

Semua sub-perintah motor dikirim dengan `cmd = 61`, dibedakan lewat `args[0]` (`motor_cmd_t`):

| args[0] | Sub-perintah |
|---|---|
| `0` | `MOTOR_CMD_INIT` |
| `1` | `MOTOR_CMD_DEINIT` |
| `2` | `MOTOR_CMD_SET_DIRECTION` |
| `3` | `MOTOR_CMD_SET_SPEED` |
| `0xFF` | `MOTOR_CMD_STATE` |

#### 5a. INIT — `args = [0, driver_type, pin_a, pin_b, pin_c, pin_d]`

`driver_type` (`args[1]`):

| Nilai | Driver | Pin yang dibutuhkan (`args[2..5]`) |
|---|---|---|
| `0` | BTS7960 | `r_en`, `rpwm`, `l_en`, `lpwm` |
| `1` | DRV8833 | `stby`, `in1`, `in2` (args[5] tidak dipakai) |
| `2` | TB6612FNG | `stby`, `in1`, `in2`, `pwm` |
| `3` | ZK5AD | `pwm_a`, `pwm_b` (args[4], args[5] tidak dipakai) |

Firmware otomatis mengalokasikan **slot motor** yang kosong (maksimum `MOTOR_MAX_COUNT = CMD_MOTOR_END - CMD_MOTOR` slot). `motor_id` hasil alokasi (mulai dari `1`) dikembalikan di `data[0]`, dan **`cmd` pada balasan menjadi `61 + motor_id`** (mis. motor pertama → cmd balasan `62`) sehingga host bisa membedakan balasan tiap motor tanpa perlu menyimpan mapping tambahan.

Balasan (`data`): `[motor_id, active(0/1), direction, speed]`.

#### 5b. DEINIT — `args = [1, motor_id]`

Melepas slot motor (agar bisa dipakai driver lain). Balasan sama formatnya seperti INIT.

#### 5c. SET_DIRECTION — `args = [2, motor_id, direction]`

`direction` (`motor_direction_t`):

| Nilai | Arti |
|---|---|
| `0` | COAST (bebas berputar) |
| `1` | FORWARD (maju) |
| `2` | REVERSE (mundur) |
| `3` | BRAKE (rem aktif) |

#### 5d. SET_SPEED — `args = [3, motor_id, speed]`

`speed` = 0–100 (persen duty cycle PWM, nilai di atas 100 otomatis dibatasi ke 100).

#### 5e. STATE — `args = [0xFF, motor_id]`

Membaca status slot motor tanpa mengubah apapun. Balasan sama formatnya seperti INIT/DEINIT.

---

## Contoh Penggunaan

Contoh berikut mengirim paket mentah lewat port serial (`/dev/ttyACM0`) — cocok untuk uji cepat dari terminal Linux. Untuk aplikasi produksi (mis. control panel berbasis browser seperti `pico-control-panel.html`), bungkus byte yang sama ke dalam pesan biner sesuai jalur transport yang dipakai (WebSerial, WebUSB Vendor interface, atau jembatan WebSocket↔Serial).

### Inisialisasi motor BTS7960 (menjadi motor pertama)

```bash
printf '\x00\x3d\x00\x00\xff\x0f\xff\x0e' > /dev/ttyACM0
```

Penjelasan byte: `id=0x00`, `cmd=0x3d (61, CMD_MOTOR)`, `args=[0x00 INIT, 0x00 BTS7960, 0xff r_en, 0x0f rpwm, 0xff l_en, 0x0e lpwm]`

### Set arah motor ke FORWARD (motor_id = 1)

```bash
printf '\x00\x3d\x02\x01\x01\x0f\xff\x0e' > /dev/ttyACM0
```

`args = [SET_DIRECTION(2), motor_id=1, direction=FORWARD(1), ...]`

### Set kecepatan motor ke 10% (motor_id = 1)

```bash
printf '\x00\x3d\x03\x01\x0a\x00\x00\x00' > /dev/ttyACM0
```

`args = [SET_SPEED(3), motor_id=1, speed=10 (0x0a), ...]`

### Nyalakan LED onboard

```bash
printf '\x00\x15\x01\x00\x00\x00\x00\x00' > /dev/ttyACM0
```

`cmd=0x15 (21, CMD_LED_DEFAULT)`, `args[0]=1 (ON)`

### Set warna WS2812 ke merah penuh

```bash
printf '\x00\x16\x00\x01\x00\x00\x00\x00' > /dev/ttyACM0   # init dulu
printf '\x00\x16\x01\xff\x00\x00\x00\x00' > /dev/ttyACM0   # set RGB = 255,0,0
```

`cmd=0x16 (22, CMD_LED_WS2812_1BIT)` — paket pertama init PIO, paket kedua set warna merah penuh.

### Masuk mode BOOTSEL dari software

```bash
make bootsel TTYACM=/dev/ttyACM0
```

> **Catatan:** contoh `printf` di atas mengasumsikan port serial punya izin akses (lihat [Troubleshooting](#troubleshooting) soal permission), dan device sedang menerima paket lewat jalur CDC (`LIB_PICO_STDIO_USB` didefinisikan saat build). Untuk kontrol dari aplikasi web/browser dengan trafik tinggi (mis. joystick real-time), gunakan interface **Vendor** (WebUSB) yang lebih cocok untuk data biner terus-menerus tanpa konflik dengan log teks.

---

## Monitoring / Debug

Untuk melihat log serial (jika `printf` debug diaktifkan, mis. dengan meng-uncomment `packet_rx_log(&rx);` / `packet_tx_log(&tx_buf[tx_count]);` di `src/app/main.c` dan build dengan stdio-USB aktif):

```bash
make monitor TTYACM=/dev/ttyACM0
```

Ini menjalankan:
```bash
stty -F /dev/ttyACM0 raw -echo 115200
cat /dev/ttyACM0
```

Bisa juga memakai `picotool info -a` (dijalankan otomatis setelah `make flash`) untuk melihat metadata program: versi, deskripsi, pinout power, dan fitur USB yang aktif — semua ini didefinisikan lewat `bi_decl(...)` di `src/config/config_usb.h`.

---

## Troubleshooting

| Masalah | Solusi |
|---|---|
| `Permission denied` saat akses `/dev/ttyACM0` | Tambahkan user ke grup `dialout` (`sudo usermod -aG dialout $USER`, lalu re-login), atau jalankan perintah dengan `sudo`. |
| `make bootsel` gagal, `$(TTYACM) not found` | Pastikan device sudah ter-flash & terhubung, cek `ls /dev/ttyACM*` untuk device path yang benar, lalu override `TTYACM=/dev/ttyACMx`. |
| Build gagal, `pico_sdk_init.cmake` tidak ditemukan | Pastikan `lib/pico-sdk` ada dan valid (lihat [Menyiapkan Pico SDK](#menyiapkan-pico-sdk)), atau set variabel `PICO_SDK_PATH`. |
| `picotool load` gagal, board tidak terdeteksi | Pastikan board dalam mode BOOTSEL (tahan tombol BOOTSEL saat colok USB), lalu ulangi `make flash`. |
| WS2812 tidak menyala / warna salah | Pastikan sudah kirim paket init (`args=[0,1,...]`) sebelum set warna, dan cek potensi konflik pin dengan UART0 TX (keduanya di `PIN_21`/GP16 pada konfigurasi default). |
| Motor tidak merespons | Pastikan `MOTOR_CMD_INIT` sukses (`status=OK`) dan gunakan `motor_id` yang dikembalikan firmware — jangan asumsikan selalu `1` jika ada slot lain yang sudah dipakai duluan. Cek juga apakah semua slot (`MOTOR_MAX_COUNT`) sudah terpakai penuh. |
| Tidak ada balasan dari device sama sekali | Pastikan panjang paket **selalu tepat 8 byte**. Paket lebih pendek dari `id+cmd` (2 byte) akan diabaikan firmware, dan paket dengan panjang tidak selaras kelipatan 8 byte bisa membuat parsing bergeser. |

---

## Lisensi

Belum ditentukan pada repo ini — tambahkan file `LICENSE` sesuai kebutuhan sebelum distribusi.