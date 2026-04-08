#include "app/dispatcher.h"
#include "bsp/board.h"
#include "config/config_usb.h"
#include "hal/uart.h"
#include "handlers/led/led_default.h"
#include "hardware/clocks.h"
#include "packet/rx/packet_rx.h"
#include "packet/rx/packet_rx_log.h"
#include "packet/rx/packet_rx_process.h"
#include "packet/tx/packet_tx.h"
#include "packet/tx/packet_tx_log.h"
#include "pico/platform/compiler.h"
#include "pico/stdlib.h"
#include "uart/uart_0.h"

int main(void) {
    set_sys_clock_khz(133000, true);  // MIN=50000, MAX=180000

    // clang-format off
    #if CFG_TUD_ENABLED
        tusb_init();
    #endif
    // clang-format on

    // clang-format off
    #if defined(LIB_PICO_STDIO_USB)
        stdio_usb_init();
    #endif
    // clang-format on

    led_default_init();

    uart_0_init(PIN_22, PIN_21, PICO_DEFAULT_UART_BAUD_RATE, uart_0_irq_handler);

    static packet_tx_t tx_buf[PACKET_RX_EP_CAPACITY];

    while (true) {
        // clang-format off
        #if CFG_TUD_ENABLED
            tud_task();
        #endif
        // clang-format on

        packet_rx_t rx;
        uint32_t tx_count = 0;
        while (packet_rx_rb_pop(&rx)) {
            tx_buf[tx_count++] = dispatch(rx);
            if (__builtin_expect(tx_count == PACKET_RX_EP_CAPACITY, 0)) break;
        }

        if (!tx_count) continue;

        uint32_t packet_tx_size = tx_count * sizeof(packet_tx_t);

        // clang-format off
        #if UART_0_ENABLE
            uart_write_blocking(UART_0_ID, (const uint8_t*)tx_buf, packet_tx_size);
        #endif

        #if CFG_TUD_VENDOR
            if (tud_vendor_mounted()) {
                uint32_t available = tud_vendor_write_available(),
                         tx_size   = TU_MIN(packet_tx_size, available) & ~(sizeof(packet_tx_t) - 1);
                if (tx_size > 0 && tud_vendor_write(tx_buf, tx_size))
                    tud_vendor_write_flush();
            }
        #endif

        #if CFG_TUD_HID
            if (tud_hid_ready())
                tud_hid_report(0, tx_buf, INPUT_HID_REPORT_COUNT);
        #endif
        // clang-format on
    }
}