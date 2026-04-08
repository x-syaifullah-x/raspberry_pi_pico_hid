# VOID COMMANDER 🎮
> Control anything, from anywhere, through the void.

Sistem kendali kendaraan multi-platform berbasis **Raspberry Pi Pico** yang dapat dikontrol melalui Browser, HP, maupun Remote fisik melalui server lokal.

---

## 🏗️ Arsitektur Sistem

```
[Browser / HP]  || [Remote Fisik*]
       ↓
   [Commander Server]  ← berjalan di Mini PC / STB
       ↓
   [Raspberry Pi Pico]  ← terhubung via USB
```

> *Remote fisik belum diimplementasi

---

## 📦 Hardware yang Dibutuhkan

- Raspberry Pi Pico RP2040
- Motor Driver **TB6612FNG**
- Dinamo 2 unit
- Mini PC / STB (sebagai server)
- Kabel USB (Pico → Mini PC)

---

---

## 🎛️ Fitur Pico

### 💡 LED Onboard

| Fungsi | Keterangan |
|--------|------------|
| `LED ON` | Nyalakan LED |
| `LED OFF` | Matikan LED |
| `LED BLINK` | Kedip dengan delay yang dapat diatur |

### 📊 ADC Onboard

| Fungsi | Keterangan |
|--------|------------|
| `READ ADC` | Baca nilai ADC onboard Pico |

### ⚙️ Kontrol Motor (TB6612FNG)

| Tombol | Motor A | Motor B | Fungsi |
|--------|---------|---------|--------|
| ⬆️ Maju | Maju | Maju | Kendaraan maju |
| ⬇️ Mundur | Mundur | Mundur | Kendaraan mundur |
| ⬅️ Kiri | Berhenti / Mundur | Maju | Belok kiri |
| ➡️ Kanan | Maju | Berhenti / Mundur | Belok kanan |
| ⏹️ Stop | Berhenti | Berhenti | Kendaraan berhenti |

> ⚠️ Logika belok dapat disesuaikan tergantung posisi pemasangan motor.

---

## 🚀 Instalasi

> ⚠️ **Penting:** Raspberry Pi Pico harus dipasang dan siap terlebih dahulu sebelum server dijalankan.

### Clone Repository

```bash
git clone https://github.com/x-syaifullah-x/raspberry_pi_pico_commander.git
cd pi_pico_commander
```

---

### Langkah 1 — Instalasi Raspberry Pi Pico

#### 1. Build Binary Pico

```bash
make pi_pico
```

Binary akan tersedia di:
```
build/pi_pico/pi_pico.uf2
```

#### 2. Upload ke Pico

1. Tekan dan tahan tombol **BOOTSEL** pada Pico
2. Hubungkan Pico ke PC via kabel USB (tetap tahan BOOTSEL)
3. Lepas tombol BOOTSEL setelah Pico terdeteksi sebagai drive **RPI-RP2**
4. Salin file `build/pi_pico/pi_pico.uf2` ke drive **RPI-RP2**
5. Pico akan restart otomatis dan siap digunakan

---

### Langkah 2 — Instalasi Server

#### 1. Build Server

```bash
make commander
```

Binary akan tersedia di:
```
build/commander/pi_pico_commander
```

#### 2. Jalankan Server

```bash
./build/commander/pi_pico_commander
```

Server akan berjalan dan siap menerima koneksi dari browser atau HP.

---

## 🌐 Menggunakan Web Controller

### Dari Browser (PC)

1. Pastikan Pico sudah terhubung dan server sudah berjalan
2. Buka browser dan akses:
```
http://localhost:8080
```

### Dari HP

1. Pastikan HP dan Mini PC berada di **jaringan WiFi yang sama**
2. Cek IP address Mini PC:
```bash
ip addr show
```
3. Buka browser HP dan akses:
```
http://<IP_MINI_PC>:8080
```

---

## 🛠️ Development

### Makefile Commands

```bash
make pi_pico      # Build binary Pico
make commander    # Build server
make clean        # Hapus build
make run          # Build & jalankan
```

---

## 🗺️ Roadmap

- [x] Web controller (Browser & HP)
- [x] Koneksi USB ke Pico
- [x] Kontrol motor via TB6612FNG
- [x] LED onboard (ON / OFF / BLINK)
- [x] Read ADC onboard
- [ ] Remote fisik (HID)
- [ ] Live telemetry (kecepatan, baterai)
- [ ] Mode otomatis / autonomous
- [ ] Support kendaraan tambahan

---

## 📄 Lisensi

MIT License © 2026 VoidCommander

---

*VoidCommander — Control anything, from anywhere, through the void* 🔥