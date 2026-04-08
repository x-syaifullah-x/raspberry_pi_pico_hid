#pragma once

#include "esp_http_server.h"

#ifdef __cplusplus
extern "C" {
#endif

static esp_err_t server_api_wifi_ap_handler(httpd_req_t* req);

static const httpd_uri_t server_api_wifi_ap_uri_status = {
    .uri = "/api/wifi/ap/status",
    .method = HTTP_POST,
    .handler = server_api_wifi_ap_handler,
    .user_ctx = NULL,
};

static const httpd_uri_t server_api_wifi_ap_uris[] = {
    server_api_wifi_ap_uri_status,
};

#ifdef __cplusplus
}
#endif