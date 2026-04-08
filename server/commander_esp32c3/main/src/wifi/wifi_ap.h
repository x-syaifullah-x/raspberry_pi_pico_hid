#pragma once

#include <string.h>

#include "esp_err.h"
#include "esp_netif_types.h"
#include "esp_wifi_types_generic.h"

#ifdef __cplusplus
extern "C" {
#endif

static const char* TAG_AP = "WiFi SoftAP";

static esp_netif_t* wifi_init_softap(void) {
    esp_netif_t* esp_netif_ap = esp_netif_create_default_wifi_ap();
    wifi_config_t wifi_ap_config = {
        .ap = {
            .ssid = CONFIG_ESP_WIFI_AP_SSID,
            .ssid_len = strlen(CONFIG_ESP_WIFI_AP_SSID),
            .channel = CONFIG_ESP_WIFI_AP_CHANNEL,
            .password = CONFIG_ESP_WIFI_AP_PASSWORD,
            .max_connection = CONFIG_ESP_MAX_STA_CONN_AP,
            .authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {.required = false},
        },
    };
    if (strlen(CONFIG_ESP_WIFI_AP_PASSWORD) == 0)
        wifi_ap_config.ap.authmode = WIFI_AUTH_OPEN;

    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_AP, &wifi_ap_config));
    ESP_LOGI(TAG_AP, "wifi_init_softap finished. SSID:%s channel:%d", CONFIG_ESP_WIFI_AP_SSID, CONFIG_ESP_WIFI_AP_CHANNEL);
    return esp_netif_ap;
}

#ifdef __cplusplus
}
#endif