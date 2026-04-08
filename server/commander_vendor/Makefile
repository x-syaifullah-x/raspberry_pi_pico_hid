.PHONY: $(MAKECMDGOALS)

SHELL := /bin/bash

PROJECT_NAME = commander-vendor
ROOT_DIR := $(patsubst %/,%,$(dir $(abspath $(lastword $(MAKEFILE_LIST)))))
BUILD_DIR = $(ROOT_DIR)/build
SRC = $(ROOT_DIR)/src

ID_VENDOR = 1902
ID_PRODUCT = 1992
SERIAL_NUMBER = E0C9125B0D9B
USER = xxx

BINARY = $(BUILD_DIR)/$(PROJECT_NAME) --vid=0x$(ID_VENDOR) --pid=0x$(ID_PRODUCT)

RULES_FILE = 99-$(PROJECT_NAME).rules
RULES_DST  = /etc/udev/rules.d/$(RULES_FILE)

LIBUSB_DIR = /tmp/libusb
LIBUSB_ARCH ?=x86_64

GOOS   := $(shell go env GOOS)
GOARCH := $(shell go env GOARCH)

dependencies:
	apt install --no-install-suggests --no-install-recommends ca-certificates gcc libc-dev pkg-config libusb-1.0-0-dev -y

rules_setup:
	@echo "Installing udev rules..."
	@printf 'ACTION=="add", SUBSYSTEM=="usb", ATTRS{idVendor}=="$(ID_VENDOR)", ATTRS{idProduct}=="$(ID_PRODUCT)", ATTRS{serial}=="$(SERIAL_NUMBER)", MODE="0666", GROUP="$(USER)", RUN+="/bin/systemd-run --unit=$(PROJECT_NAME) $(BINARY)"' | tee $(RULES_DST)
	udevadm control --reload-rules && udevadm trigger --subsystem-match=usb
	@echo "Done. Verifying..."

rules_remove:
	rm -f $(RULES_DST)
	udevadm control --reload-rules && udevadm trigger --subsystem-match=usb
	@echo "Rules removed."

build:
	ARCH=$(GOARCH) OS=$(GOOS) CGO_ENABLED=1 CC=gcc go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME)-${GOOS}-${GOARCH} $(SRC)

build_libusb:
	apt install --no-install-suggests --no-install-recommends git openssh-client automake libtool g++-aarch64-linux-gnu -y
	git clone https://github.com/libusb/libusb.git $(LIBUSB_DIR) --depth=1|| true
	cd $(LIBUSB_DIR) && ./autogen.sh --disable-udev --enable-static --disable-shared --host=$(LIBUSB_ARCH)-linux-gnu --prefix=$$HOME/.local/$(LIBUSB_ARCH)
	cd $(LIBUSB_DIR) && make -j$(nproc)
	cd $(LIBUSB_DIR) && make install
	cd $(LIBUSB_DIR) && make clean &>/dev/null

# 	GOOS=linux GOARCH=arm64 CC=aarch64-linux-gnu-gcc CGO_ENABLED=1 PKG_CONFIG_PATH="$$HOME/.local/arm64/lib/pkgconfig" PKG_CONFIG_LIBDIR="$$HOME/.local/arm64/lib/pkgconfig" CGO_CFLAGS="-I/$$HOME/.local/arm64/include/libusb-1.0" CGO_LDFLAGS="-L/$$HOME/.local/arm64/lib -lusb-1.0 -lpthread" go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME)-linux-arm64 $(SRC)
# 	GOOS=linux GOARCH=amd64 CC=gcc CGO_ENABLED=1 PKG_CONFIG_PATH="$$HOME/.local/amd64/lib/pkgconfig" PKG_CONFIG_LIBDIR="$$HOME/.local/amd64/lib/pkgconfig" CGO_CFLAGS="-I/$$HOME/.local/amd64/include/libusb-1.0" CGO_LDFLAGS="-L/$$HOME/.local/amd64/lib -lusb-1.0 -lpthread" go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME)-linux-amd64 $(SRC)

# build_libusb:
# 	apt install --no-install-suggests --no-install-recommends git openssh-client automake libtool -y
# 	git clone https://github.com/libusb/libusb.git $(LIBUSB_DIR) --depth=1|| true
# 	cd $(LIBUSB_DIR) && ./autogen.sh --disable-udev --enable-static --disable-shared --prefix=$$HOME/.local
# 	cd $(LIBUSB_DIR) && make -j$(nproc)
# 	cd $(LIBUSB_DIR) && make install

GOARCH_aarch64 = arm64
GOARCH_x86_64  = amd64

GO_ARCH = $(GOARCH_$(LIBUSB_ARCH))

build_static: build_libusb
	GOOS=linux GOARCH=$(GO_ARCH) CC=$(LIBUSB_ARCH)-linux-gnu-gcc CGO_ENABLED=1 PKG_CONFIG_PATH="$$HOME/.local/$(LIBUSB_ARCH)/lib/pkgconfig" PKG_CONFIG_LIBDIR="$$HOME/.local/$(LIBUSB_ARCH)/lib/pkgconfig" CGO_CFLAGS="-I/$$HOME/.local/$(LIBUSB_ARCH)/include/libusb-1.0" CGO_LDFLAGS="-L/$$HOME/.local/$(LIBUSB_ARCH)/lib -lusb-1.0 -lpthread" go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME)-linux-$(LIBUSB_ARCH) $(SRC)
# 	ARCH=$(GOARCH) OS=$(GOOS) CC=gcc CGO_ENABLED=1 CGO_CFLAGS="-I/$$HOME/.local/include/libusb-1.0" CGO_LDFLAGS="-L/$$HOME/.local/lib -lusb-1.0 -lpthread" go build -buildvcs=false -ldflags='-s -w' -trimpath -o $(BUILD_DIR)/$(PROJECT_NAME)-${GOOS}-${GOARCH} $(SRC)

build_run: build
	su -c "$(BUILD_DIR)/$(PROJECT_NAME)-${GOOS}-${GOARCH} --vid=0x$(ID_VENDOR) --pid=0x$(ID_PRODUCT)"

run:
	CGO_ENABLED=1 CC=gcc go run -ldflags='-s -w' -trimpath $(SRC) --vid=0x$(ID_VENDOR) --pid=0x$(ID_PRODUCT)

run_binary:
	su -c "$(BUILD_DIR)/$(PROJECT_NAME)-${GOOS}-${GOARCH} --vid=0x$(ID_VENDOR) --pid=0x$(ID_PRODUCT)"

clean:
	rm -rfv $(BUILD_DIR)