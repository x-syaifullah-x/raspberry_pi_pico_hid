#!/bin/sh

CURRENT_DIR="/$(realpath --relative-to=/ $(dirname $0))"

su -c "printf '\xFF\xFF\x01' > /dev/hidraw0" # REBOOT_BOOTSEL

cmake -S . -B build
cmake --build build --target raspberry_pi_pico_hid
cp -v build/raspberry_pi_pico_hid.uf2 /media/$USER/RPI-RP2