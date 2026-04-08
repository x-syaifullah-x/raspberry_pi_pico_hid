cmake_minimum_required(VERSION 3.13)

set(CMAKE_BUILD_TYPE Release)

include(lib/pico-sdk/pico_sdk_init.cmake)

set(PICO_FIRMWARE controller)
get_filename_component(PROJECT_NAME "${PICO_FIRMWARE}" NAME)
project(${PROJECT_NAME} C CXX ASM)

pico_sdk_init()

add_compile_options(
    -O3
    -ffunction-sections
    -fdata-sections
    -Wall        # ← semua warning umum
    -Wextra      # ← warning tambahan
    # -Wpedantic   # ← warning strict standard C
)

add_link_options(-Wl,--gc-sections)

file(GLOB_RECURSE SOURCES "${CMAKE_CURRENT_LIST_DIR}/src/*.c")

add_executable(
    ${PROJECT_NAME}
	${SOURCES}
)

# target_compile_options(${PROJECT_NAME} PRIVATE
#     -Wswitch
#     -Wswitch-enum
# )

target_compile_definitions(${PROJECT_NAME} PRIVATE
    USB_ID_VENDOR=0x1902
    USB_ID_PRODUCT=0x1992
    USB_MANUFACTIRER=\"Raspberry\ PI\ Pico\"
    USB_PRODUCT=\"RP2040\"
    USB_SERIAL_NUMBER=\"E0C9125B0D9B\"
)
    
target_link_libraries(
    ${PROJECT_NAME}
    hardware_adc
    hardware_dma
    hardware_gpio
    hardware_irq
    hardware_pwm
    hardware_uart
    hardware_pio
    pico_stdlib
    # pico_multicore
    tinyusb_board
    tinyusb_device
)

# target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_include_directories(${PROJECT_NAME} PRIVATE src/)

# target_compile_options(${PROJECT_NAME} PRIVATE -ffunction-sections -fdata-sections)

# target_link_options(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)

pico_set_binary_type(${PROJECT_NAME} copy_to_ram)

pico_add_extra_outputs(${PROJECT_NAME})

# disable stdio routing → USB
pico_enable_stdio_usb(${PROJECT_NAME} 0)

# disable stdio routing → UART
pico_enable_stdio_uart(${PROJECT_NAME} 0)
