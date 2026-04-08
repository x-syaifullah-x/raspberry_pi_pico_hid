.PHONY: $(MAKECMDGOALS)

SHELL := /bin/bash

PROJECT_NAME := $(notdir $(CURDIR))
ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR = $(ROOT_DIR)/build
FIRMWARE_UF2 = $(BUILD_DIR)/controller.uf2

TTYACM ?= /dev/ttyACM0

monitor:
	stty -F $(TTYACM) raw -echo 115200
	cat $(TTYACM)

picoboot:
	@if [ ! -e $(TTYACM) ]; then \
		echo "Error: $(TTYACM) not found"; \
		exit 1; \
	fi
	printf '\xFF\x02\x01\x00\x00\x00\x00\x00' > $(TTYACM)

dependencies:
	apt install --no-install-suggests --no-install-recommends -y cmake gcc-arm-none-eabi picotool libstdc++-arm-none-eabi-newlib python3

configure:
	cmake -S $(ROOT_DIR) -B $(BUILD_DIR)

build:
	cmake --build $(BUILD_DIR)

# flash:
# 	cp -rfv $(FIRMWARE_UF2) /media/$$USER/RPI-RP2/

flash:
	picotool load $(FIRMWARE_UF2)
	picotool info -a

flash_and_reboot: flash
	picotool reboot

clean:
	rm -rfv $(BUILD_DIR)