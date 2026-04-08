#pragma once

#include "cJSON.h"
#include "esp_http_server.h"
#include "esp_wifi.h"
#include "server_api_wifi_sta_uri.h"

#ifdef __cplusplus
extern "C" {
#endif

static esp_err_t server_api_wifi_sta_handler(httpd_req_t* req) {
    if (strcmp(req->uri, server_api_wifi_sta_uri_scan.uri) == 0) {
        wifi_scan_config_t scan_config = {
            .ssid = NULL,
            .bssid = NULL,
            .channel = 0,
            .show_hidden = false,
        };

        ESP_ERROR_CHECK(esp_wifi_scan_start(&scan_config, true));

        uint16_t ap_count = 0;
        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_num(&ap_count));

        wifi_ap_record_t* ap_records = malloc(sizeof(wifi_ap_record_t) * ap_count);

        ESP_ERROR_CHECK(esp_wifi_scan_get_ap_records(&ap_count, ap_records));

        cJSON* root = cJSON_CreateArray();

        for (int i = 0; i < ap_count; i++) {
            cJSON* ap = cJSON_CreateObject();
            cJSON_AddStringToObject(ap, "ssid", (char*)ap_records[i].ssid);
            cJSON_AddNumberToObject(ap, "authmode", ap_records[i].authmode);
            cJSON_AddNumberToObject(ap, "rssi", ap_records[i].rssi);
            cJSON_AddItemToArray(root, ap);
        }

        free(ap_records);

        char* json = cJSON_PrintUnformatted(root);

        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);

        cJSON_free(json);
        cJSON_Delete(root);

        return ESP_OK;
    }

    if (strcmp(req->uri, server_api_wifi_sta_uri_connect.uri) == 0) {
        char buf[256];
        if (req->content_len >= sizeof(buf)) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Request too large");
            return ESP_FAIL;
        }

        int received = httpd_req_recv(req, buf, req->content_len);
        if (received <= 0)
            return ESP_FAIL;

        cJSON* root = cJSON_Parse(buf);

        const cJSON* ssid = cJSON_GetObjectItem(root, "ssid");
        const cJSON* password = cJSON_GetObjectItem(root, "password");
        const cJSON* authmode = cJSON_GetObjectItem(root, "authmode");

        esp_wifi_disconnect();
        wifi_config_t wifi_sta_config = {0};
        strncpy((char*)wifi_sta_config.sta.ssid, ssid->valuestring, sizeof(wifi_sta_config.sta.ssid) - 1);
        strncpy((char*)wifi_sta_config.sta.password, password->valuestring, sizeof(wifi_sta_config.sta.password) - 1);
        wifi_sta_config.sta.threshold.authmode = authmode->valueint;
        if (authmode->valueint == WIFI_AUTH_WPA3_PSK || authmode->valueint == WIFI_AUTH_WPA2_WPA3_PSK) {
            wifi_sta_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;
        }
        // wifi_sta_config.sta.sae_pwe_h2e = WPA3_SAE_PWE_BOTH;

        ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));
        esp_wifi_connect();

        cJSON_Delete(root);

        return ESP_OK;
    }

    if (strcmp(req->uri, server_api_wifi_sta_uri_disconnect.uri) == 0) {
        esp_wifi_disconnect();
        return ESP_OK;
    }

    if (strcmp(req->uri, server_api_wifi_sta_uri_status.uri) == 0) {
        wifi_ap_record_t ap_info;
        esp_wifi_sta_get_ap_info(&ap_info);
        esp_netif_t* sta_netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        esp_netif_ip_info_t esp_netif_ip_info;
        esp_netif_get_ip_info(sta_netif, &esp_netif_ip_info);

        cJSON* status = cJSON_CreateObject();
        cJSON_AddStringToObject(status, "ssid", (char*)ap_info.ssid);

        char ip_str[16];
        esp_ip4addr_ntoa(&esp_netif_ip_info.ip, ip_str, sizeof(ip_str));
        cJSON_AddStringToObject(status, "ip", ip_str);

        char* json = cJSON_PrintUnformatted(status);

        httpd_resp_set_type(req, "application/json");
        httpd_resp_send(req, json, HTTPD_RESP_USE_STRLEN);

        cJSON_free(json);
        cJSON_Delete(status);

        return ESP_OK;
    }

    return ESP_FAIL;
}

#ifdef __cplusplus
}
#endif