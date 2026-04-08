#!/bin/sh

CURRENT_DIR="/$(realpath --relative-to=/ $(dirname $0))"
PROJECT_NAME="raspberry-pi-pico-hid"
ID_VENDOR=$(grep -oP '\.idVendor\s*=\s*0x\K[0-9A-Fa-f]+' $CURRENT_DIR/../../src/usb_descriptors.c)
ID_PRODUCT=$(grep -oP '\.idProduct\s*=\s*0x\K[0-9A-Fa-f]+' $CURRENT_DIR/../../src/usb_descriptors.c)

tee "/etc/udev/rules.d/${PROJECT_NAME}.rules" << EOF
ACTION=="add", SUBSYSTEM=="hidraw", ATTRS{idVendor}=="$ID_VENDOR", ATTRS{idProduct}=="$ID_PRODUCT", TAG+="systemd", ENV{SYSTEMD_WANTS}+="${PROJECT_NAME}@%k.service"
EOF

echo ""

tee "/lib/systemd/system/${PROJECT_NAME}@.service"<< EOF
[Unit]
Description=Raspberry Pi Pico HID (%i)
After=dev-%i.device
BindsTo=dev-%i.device

[Service]
Type=simple
ExecStart=/media/xxx/sdc2/xxx/Git/x-syaifullah-x/raspberry_pi_pico_hid/server/build/raspbarry_pi_pico_hid_server /dev/%i
Restart=no

[Install]
WantedBy=multi-user.target
EOF