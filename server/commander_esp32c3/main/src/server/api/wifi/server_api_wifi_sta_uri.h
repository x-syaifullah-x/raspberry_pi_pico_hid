#pragma once

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

static esp_err_t server_api_wifi_sta_handler(httpd_req_t* req);

static const httpd_uri_t server_api_wifi_sta_uri_scan = {
    .uri = "/api/wifi/sta/scan",
    .method = HTTP_POST,
    .handler = server_api_wifi_sta_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t server_api_wifi_sta_uri_connect = {
    .uri = "/api/wifi/sta/connect",
    .method = HTTP_POST,
    .handler = server_api_wifi_sta_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t server_api_wifi_sta_uri_disconnect = {
    .uri = "/api/wifi/sta/disconnect",
    .method = HTTP_POST,
    .handler = server_api_wifi_sta_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t server_api_wifi_sta_uri_status = {
    .uri = "/api/wifi/sta/status",
    .method = HTTP_POST,
    .handler = server_api_wifi_sta_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t server_api_wifi_sta_uris[] = {
    server_api_wifi_sta_uri_scan,
    server_api_wifi_sta_uri_connect,
    server_api_wifi_sta_uri_disconnect,
    server_api_wifi_sta_uri_status,
};

#ifdef __cplusplus
}
#endif