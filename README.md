# Raspberry Pi Pico HID

A simple and minimal USB HID project for Raspberry Pi Pico using TinyUSB.

---

## 🚀 Clone

Clone the repository along with its submodules (shallow for faster download):

```sh
git clone --recurse-submodules --depth 1 --shallow-submodules ssh://git@github.com/x-syaifullah-x/raspberry_pi_pico_hid
cd raspberry_pi_pico_hid
```

---

## 🛠️ Build

Build the project using CMake:

```sh
cmake -S . -B build
cmake --build build --target raspberry_pi_pico_hid
```

---

## 📦 Flash

1. Put your Raspberry Pi Pico into **BOOTSEL mode**
2. Copy the generated `.uf2` file:

```sh
# REBOOT BOOTSEL
printf '\xFF\x01' > /dev/hidraw0
cp -v build/raspberry_pi_pico_hid.uf2 /media/$USER/RPI-RP2
```

The device will automatically reboot after flashing.

---

## ⚡ Features

* USB HID implementation using TinyUSB
* Minimal and lightweight codebase
* Fast cloning with shallow submodules
* Easy build and flash process

---

## 🧩 Submodules

If you forgot to clone submodules:

```sh
git submodule update --init --recursive --depth 1
```

---

## 📁 Output

After building, the firmware will be available at:

```sh
build/raspberry_pi_pico_hid.uf2
```

---

## 📝 Notes

* Make sure the Pico is mounted as `RPI-RP2`
* Requires CMake and Raspberry Pi Pico SDK
* Tested on Linux environment
* Submodules are required for successful build

---

## 🧪 Tips

* Use `--depth 1` to speed up cloning
* Use `--shallow-submodules` to reduce submodule size
* You can automate flashing with a simple script if needed

---

## 📄 License

This project is licensed under the MIT License.

## WIRING
