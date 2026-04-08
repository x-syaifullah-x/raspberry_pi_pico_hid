#pragma once

#include "server/api/wifi/server_api_wifi_ap.h"
#include "server/api/wifi/server_api_wifi_sta.h"
#include "server/ws/server_ws.h"

#ifdef __cplusplus
extern "C" {
#endif

static esp_err_t server_options_handler(httpd_req_t* req) {
    if (req->method == HTTP_OPTIONS) {
        httpd_resp_set_hdr(req, "Access-Control-Allow-Origin", "*");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Methods", "GET, POST, OPTIONS");
        httpd_resp_set_hdr(req, "Access-Control-Allow-Headers", "Content-Type");

        httpd_resp_send(req, NULL, 0);
    }

    return ESP_OK;
}

static const httpd_uri_t server_api_uri_option = {
    .uri = "*",
    .method = HTTP_OPTIONS,
    .handler = server_options_handler,
};

static httpd_handle_t server_start(const uint16_t port) {
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.server_port = port;
    config.max_open_sockets = CONFIG_WS_MAX_CLIENTS + 2; /* +2 untuk margin HTTP biasa */
    config.uri_match_fn = httpd_uri_match_wildcard;

    httpd_handle_t server = NULL;
    if (httpd_start(&server, &config) != ESP_OK) {
        ESP_LOGE(TAG_WS, "Failed to start httpd");
        return NULL;
    }

    httpd_register_uri_handler(server, &server_api_uri_option);

    s_ws_mutex = xSemaphoreCreateMutex();
    configASSERT(s_ws_mutex);
    httpd_register_uri_handler(server, &server_uri_ws);
    ESP_LOGI(TAG_WS, "WebSocket server started → ws://[ip]%s", server_uri_ws.uri);

    for (size_t i = 0; i < sizeof(server_api_wifi_ap_uris) / sizeof(server_api_wifi_ap_uris[0]); i++) {
        httpd_register_uri_handler(server, &server_api_wifi_ap_uris[i]);
    }

    for (size_t i = 0; i < sizeof(server_api_wifi_sta_uris) / sizeof(server_api_wifi_sta_uris[0]); i++) {
        httpd_register_uri_handler(server, &server_api_wifi_sta_uris[i]);
    }

    return server;
}

#ifdef __cplusplus
}
#endif