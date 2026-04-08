.PHONY: $(MAKECMDGOALS)

SHELL := /bin/bash

PROJECT_NAME = commander-cdc
ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR = $(ROOT_DIR)/build
SRC = $(ROOT_DIR)/src

ID_VENDOR = 1902
ID_PRODUCT = 1992
SERIAL_NUMBER = E0C9125B0D9B

SERVICE_FILE = $(PROJECT_NAME)@.service
SERVICE_DST  = /etc/systemd/system/$(SERVICE_FILE)
BINARY       = $(BUILD_DIR)/$(PROJECT_NAME) --vid=$(ID_VENDOR) --pid=$(ID_PRODUCT) --serial=$(SERIAL_NUMBER)

RULES_DIR  = /etc/udev/rules.d
RULES_FILE = 99-$(PROJECT_NAME).rules
RULES_DST  = $(RULES_DIR)/$(RULES_FILE)

TTYACM ?= /dev/ttyACM0

LIBUSB_DIR = /tmp/libusb

GOOS   := $(shell go env GOOS)
GOARCH := $(shell go env GOARCH)

dependencies:
	apt install --no-install-suggests --no-install-recommends ca-certificates gcc libc-dev -y

build:
	go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME) $(SRC)

build_run: build
	su -c "$(BUILD_DIR)/$(PROJECT_NAME) --vid=$(ID_VENDOR) --pid=$(ID_PRODUCT) --serial=$(SERIAL_NUMBER)"

build_libusb:
	apt install --no-install-suggests --no-install-recommends git openssh-client automake libtool -y
	git clone https://github.com/libusb/libusb.git $(LIBUSB_DIR) --depth=1|| true
	cd $(LIBUSB_DIR) && ./autogen.sh --disable-udev --enable-static --disable-shared --prefix=$$HOME/.local
	cd $(LIBUSB_DIR) && make -j$(nproc)
	cd $(LIBUSB_DIR) && make install

build_static: build_libusb
	ARCH=$(GOARCH) OS=$(GOOS) CC=gcc CGO_ENABLED=1 CGO_CFLAGS="-I/$$HOME/.local/include/libusb-1.0" CGO_LDFLAGS="-L/$$HOME/.local/lib -lusb-1.0 -lpthread" go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME)-${GOOS}-${GOARCH} $(SRC)

run:
	go run -ldflags='-s -w' -trimpath $(SRC) --vid=$(ID_VENDOR) --pid=$(ID_PRODUCT) --serial=$(SERIAL_NUMBER)

clean:
	rm -rfv build/commander_cdc

reboot_bootsel:
	printf '\xFF\x01\x01' > $(TTYACM)

monitor:
	stty -F $(TTYACM) 115200 raw -echo
	cat $(TTYACM)

monitor_install:
	@printf 'ACTION=="add", SUBSYSTEM=="tty", ATTRS{idVendor}=="%s", ATTRS{idProduct}=="%s", ATTRS{serial}=="%s", MODE="0666", RUN+="/bin/systemd-run --no-block /bin/sh -c '\''stty -F $$env{DEVNAME} raw -echo && cat $$env{DEVNAME} | systemd-cat -t 99-$(PROJECT_NAME)-monitor'\''"\n' $(ID_VENDOR) $(ID_PRODUCT) $(SERIAL_NUMBER) | tee $(RULES_DIR)/99-$(PROJECT_NAME)-monitor.rules
	udevadm control --reload-rules
	udevadm trigger --subsystem-match=tty

monitor_remove:
	rm -rfv $(RULES_DIR)/99-$(PROJECT_NAME)-monitor.rules
	udevadm control --reload-rules
	udevadm trigger --subsystem-match=tty

install: install-  
# 	@echo "Installing udev rules..."
# 	@printf 'ACTION=="add", SUBSYSTEM=="tty", ATTRS{idVendor}=="$(ID_VENDOR)", ATTRS{idProduct}=="$(ID_PRODUCT)", ATTRS{serial}=="$(SERIAL_NUMBER)", MODE="0666", TAG+="systemd", ENV{SYSTEMD_WANTS}+="$(PROJECT_NAME)@%%k.service"' | tee $(RULES_DST)
# 	udevadm control --reload-rules && udevadm trigger --subsystem-match=tty
# 	@echo "Done. Verifying..."

# install-service:
# 	@echo "Installing systemd service..."
# 	@printf '[Unit]\nDescription=Commander CDC (%%i)\nAfter=dev-%%i.device\nBindsTo=dev-%%i.device\n\n[Service]\nType=simple\nExecStart=$(BINARY)\nRestart=no\n\n[Install]\nWantedBy=multi-user.target\n' | tee $(SERVICE_DST)
# 	systemctl daemon-reload
# 	@echo "Service installed."

# uninstall: uninstall-service
# 	rm -f $(RULES_DST)
# 	udevadm control --reload-rules && udevadm trigger --subsystem-match=tty
# 	@echo "Rules removed."

# uninstall-service:
# 	systemctl stop $(PROJECT_NAME)@ttyACM0.service || true
# 	rm -f $(SERVICE_DST)
# 	systemctl daemon-reload
# 	@echo "Service removed."