cmake_minimum_required(VERSION 3.13)

set(CMAKE_BUILD_TYPE Release)

include(lib/pico-sdk/pico_sdk_init.cmake)

get_filename_component(PROJECT_NAME ${CMAKE_CURRENT_SOURCE_DIR} NAME)
project(${PROJECT_NAME})

project(${PROJECT_NAME} C CXX ASM)

pico_sdk_init()

add_executable(
    ${PROJECT_NAME}
	src/main.c
	src/usb_descriptors.c
)

target_link_libraries(
    ${PROJECT_NAME}
    pico_stdlib
    tinyusb_device
    hardware_adc
    hardware_pwm
    # tinyusb_board
    # hardware_pio
)

target_include_directories(${PROJECT_NAME} PRIVATE ${CMAKE_CURRENT_LIST_DIR})
target_compile_options(${PROJECT_NAME} PRIVATE -O3 -ffunction-sections -fdata-sections)
target_link_options(${PROJECT_NAME} PRIVATE -Wl,--gc-sections)
# pico_set_binary_type(${PROJECT_NAME} copy_to_ram)
pico_add_extra_outputs(${PROJECT_NAME})
