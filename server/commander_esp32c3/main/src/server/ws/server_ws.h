#pragma once

#include "esp_http_server.h"
#include "server_ws_handler.h"

#ifdef __cplusplus
extern "C" {
#endif

static const httpd_uri_t server_uri_ws = {
    .uri = CONFIG_WS_URI,
    .method = HTTP_GET,
    .handler = server_handler_ws,
    .is_websocket = true,
    .handle_ws_control_frames = true,
};

#ifdef __cplusplus
}
#endif