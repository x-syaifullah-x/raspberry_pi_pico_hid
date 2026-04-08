idf_component_register(
    SRCS "src/main.c"
    PRIV_REQUIRES esp_wifi esp_netif esp_event nvs_flash esp_http_server lwip esp_driver_gpio esp_driver_uart esp_timer
    INCLUDE_DIRS "./src"
)

add_compile_options(
    -O3
    -ffunction-sections
    -fdata-sections
)