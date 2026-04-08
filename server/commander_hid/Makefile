.PHONY: $(MAKECMDGOALS)

SHELL := /bin/bash

PROJECT_NAME = commander-hid
ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR = $(ROOT_DIR)/build
SRC = $(ROOT_DIR)/src

ID_VENDOR = 1902
ID_PRODUCT = 1992
SERIAL_NUMBER = 'E0C9125B0D9B'

SERVICE_FILE = $(PROJECT_NAME)@.service
SERVICE_DST  = /etc/systemd/system/$(SERVICE_FILE)
BINARY       = /media/xxx/DATA_EXT4/xxx/Git/x-syaifullah-x/raspberry_pi_pico_commander/build/commander_hid/commander_hid --vid=0x$(ID_VENDOR) --pid=0x$(ID_PRODUCT) --serial=$(SERIAL_NUMBER)

RULES_FILE = 99-$(PROJECT_NAME).rules
RULES_DST  = /etc/udev/rules.d/$(RULES_FILE)

LIBUSB_DIR = /tmp/libusb

HIDRAW ?= /dev/hidraw0

GOOS   := $(shell go env GOOS)
GOARCH := $(shell go env GOARCH)

dependencies:
	apt install --no-install-suggests --no-install-recommends ca-certificates gcc libc-dev libudev-dev pkg-config libusb-1.0-0-dev -y

build:
	ARCH=$(GOARCH) OS=$(GOOS) CGO_ENABLED=1 go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME)-${GOOS}-${GOARCH} $(SRC)

build_run: build
	su -c "$(BUILD_DIR)/$(PROJECT_NAME)-${GOOS}-${GOARCH} --vid=0x$(ID_VENDOR) --pid=0x$(ID_PRODUCT) --serial=$(SERIAL_NUMBER)"

build_libusb:
	apt install --no-install-suggests --no-install-recommends git openssh-client automake libtool -y
	git clone https://github.com/libusb/libusb.git $(LIBUSB_DIR) --depth=1|| true
	cd $(LIBUSB_DIR) && ./autogen.sh --disable-udev --enable-static --disable-shared --prefix=$$HOME/.local
	cd $(LIBUSB_DIR) && make -j$(nproc)
	cd $(LIBUSB_DIR) && make install

build_static: build_libusb
	ARCH=$(GOARCH) OS=$(GOOS) CC=gcc CGO_ENABLED=1 CGO_CFLAGS="-I/$$HOME/.local/include/libusb-1.0" CGO_LDFLAGS="-L/$$HOME/.local/lib -lusb-1.0 -lpthread" go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME)-${GOOS}-${GOARCH} $(SRC)

run:
	CGO_ENABLED=1 go run -ldflags='-s -w' -trimpath $(SRC) --vid=0x$(ID_VENDOR) --pid=0x$(ID_PRODUCT) --serial=$(SERIAL_NUMBER)

clean:
	rm -rfv build/commander_hid

reboot_bootsel:
	printf '\xFF\x01\x02' > $(HIDRAW)

install: install-service
	@echo "Installing udev rules..."
	@printf 'ACTION=="add", SUBSYSTEM=="hidraw", ATTRS{idVendor}=="$(ID_VENDOR)", ATTRS{idProduct}=="$(ID_PRODUCT)", ATTRS{serial}=="$(SERIAL_NUMBER)", MODE="0666", TAG+="systemd", ENV{SYSTEMD_WANTS}+="$(NAME)@%%k.service"' | tee $(RULES_DST)
	udevadm control --reload-rules && udevadm trigger --subsystem-match=hidraw
	@echo "Done. Verifying..."

install-service:
	@echo "Installing systemd service..."
	@printf '[Unit]\nDescription=Commander HID (%%i)\nAfter=dev-%%i.device\nBindsTo=dev-%%i.device\n\n[Service]\nType=simple\nExecStart=$(BINARY)\nRestart=no\n\n[Install]\nWantedBy=multi-user.target\n' | tee $(SERVICE_DST)
	systemctl daemon-reload
	@echo "Service installed."

uninstall: uninstall-service
	rm -f $(RULES_DST)
	udevadm control --reload-rules && udevadm trigger --subsystem-match=hidraw
	@echo "Rules removed."

uninstall-service:
	systemctl stop $(PROJECT_NAME)@ttyACM0.service || true
	rm -f $(SERVICE_DST)
	systemctl daemon-reload
	@echo "Service removed."