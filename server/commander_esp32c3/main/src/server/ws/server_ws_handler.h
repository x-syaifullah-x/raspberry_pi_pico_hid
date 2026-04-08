#pragma once

#include "esp_err.h"
#include "esp_http_server.h"
#include "esp_log.h"
#include "freertos/semphr.h"
#include "server_ws_state.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef enum __attribute__((packed)) {
    STATUS_OK = PICO_STATUS_OK,
    STATUS_ERR = PICO_STATUS_ERR,
    STATUS_INITIAL = 255
} status_t;

typedef enum __attribute__((packed)) {
    WS_REGISTER_ERR_TIMEOUT = -255,
    WS_REGISTER_ERR_NO_MEM,
    WS_REGISTER_ERR_FULL,
} ws_register_err_t;

typedef struct __attribute__((packed)) {
    uint8_t req_id;
} ws_session_t;

#define HEADER_SIZE 7
#define RECORD_SIZE (1 + sizeof(ws_state_t))

static const char* TAG_WS = "WebSocket";

static int s_ws_fds[CONFIG_WS_MAX_CLIENTS] = {-1};
static uint8_t s_ws_client_count = 0;

static SemaphoreHandle_t s_ws_mutex;

static int ws_register(httpd_req_t* req) {
    if (xSemaphoreTake(s_ws_mutex, pdMS_TO_TICKS(100)) != pdTRUE)
        return WS_REGISTER_ERR_TIMEOUT;

    if (s_ws_client_count >= CONFIG_WS_MAX_CLIENTS) {
        xSemaphoreGive(s_ws_mutex);
        return WS_REGISTER_ERR_FULL;
    }

    ws_session_t* session = calloc(1, sizeof(ws_session_t));
    if (!session) {
        xSemaphoreGive(s_ws_mutex);
        return WS_REGISTER_ERR_NO_MEM;
    }

    int fd = httpd_req_to_sockfd(req);

    req->sess_ctx = session;
    req->free_ctx = free;

    s_ws_fds[s_ws_client_count++] = fd;

    xSemaphoreGive(s_ws_mutex);
    return fd;
}

static void ws_unregister(httpd_req_t* req) {
    if (xSemaphoreTake(s_ws_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return;

    int fd = httpd_req_to_sockfd(req);

    for (uint8_t i = 0; i < s_ws_client_count; i++) {
        if (s_ws_fds[i] == fd) {
            s_ws_fds[i] = s_ws_fds[--s_ws_client_count];
            break;
        }
    }
    xSemaphoreGive(s_ws_mutex);
}

static void ws_close(httpd_req_t* req, const char* reason, uint16_t code) {
    size_t reason_len = strlen(reason);
    uint8_t payload[2 + 64];

    payload[0] = code >> 8;    // high byte
    payload[1] = code & 0xFF;  // low byte

    memcpy(&payload[2], reason, reason_len);

    httpd_ws_frame_t frame = {
        .type = HTTPD_WS_TYPE_CLOSE,
        .payload = payload,
        .len = 2 + reason_len,
        .final = true,
    };

    int fd = httpd_req_to_sockfd(req);
    httpd_ws_send_frame_async(req->handle, fd, &frame);
    httpd_sess_trigger_close(req->handle, fd);
}

static int ws_process(const uint8_t* buf, size_t len);

static void ws_send(httpd_handle_t server, uint8_t fd, const uint8_t* data, size_t len) {
    if (xSemaphoreTake(s_ws_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return;

    httpd_ws_frame_t frame = {
        .type = HTTPD_WS_TYPE_BINARY,
        .payload = (uint8_t*)data,
        .len = len,
        .final = true,
    };

    esp_err_t err = httpd_ws_send_frame_async(server, fd, &frame);
    if (err != ESP_OK) {
        for (uint32_t i = 0; i < s_ws_client_count; i++) {
            if (s_ws_fds[i] == fd) {
                s_ws_fds[i] = s_ws_fds[--s_ws_client_count];
                break;
            }
        }
    }

    xSemaphoreGive(s_ws_mutex);
}

static void ws_broadcast(httpd_handle_t server, const uint8_t* data, size_t len) {
    if (xSemaphoreTake(s_ws_mutex, pdMS_TO_TICKS(100)) != pdTRUE) return;

    httpd_ws_frame_t frame = {
        .type = HTTPD_WS_TYPE_BINARY,
        .payload = (uint8_t*)data,
        .len = len,
        .final = true,
    };

    for (int i = (int)s_ws_client_count - 1; i >= 0; i--) {
        esp_err_t err = httpd_ws_send_frame_async(server, s_ws_fds[i], &frame);
        if (err != ESP_OK) {
            ESP_LOGW(TAG_WS, "Send failed fd=%d err=%d, removing", s_ws_fds[i], err);
            s_ws_fds[i] = s_ws_fds[--s_ws_client_count];
        }
    }

    xSemaphoreGive(s_ws_mutex);
}

static esp_err_t server_handler_ws(httpd_req_t* req) {
    httpd_ws_frame_t frame = {0};
    httpd_ws_recv_frame(req, &frame, 0);
    uint8_t ws_rx[frame.len];
    frame.payload = ws_rx;
    httpd_ws_recv_frame(req, &frame, frame.len);

    ESP_LOGI("FRAME_TYPE", "%u", frame.type);

    switch (frame.type) {
        case HTTPD_WS_TYPE_TEXT:
        case HTTPD_WS_TYPE_BINARY:
            if (req->sess_ctx == NULL) {
                // VALIDATE RX

                int fd = ws_register(req);
                if (fd > 0) {
                    uint8_t payload[HEADER_SIZE + RECORD_SIZE * (sizeof(pico_states) / sizeof(pico_states[0]))];
                    payload[0] = STATUS_INITIAL;
                    payload[1] = fd;

                    uint8_t* p = payload + HEADER_SIZE;
                    for (uint32_t i = 0; i < sizeof(pico_states) / sizeof(pico_states[0]); i++) {
                        p[0] = PICO_STATUS_OK;
                        memcpy(&p[1], &pico_states[i], sizeof(ws_state_t));
                        p += RECORD_SIZE;
                    }

                    httpd_ws_frame_t frame = {
                        .type = HTTPD_WS_TYPE_BINARY,
                        .payload = payload,
                        .len = sizeof(payload),
                        .final = true,
                    };
                    httpd_ws_send_frame_async(req->handle, fd, &frame);
                    return ESP_OK;
                }

                switch ((ws_register_err_t)fd) {
                    case WS_REGISTER_ERR_TIMEOUT:
                        ws_close(req, "Time Out", 1013);
                        break;
                    case WS_REGISTER_ERR_NO_MEM:
                        ws_close(req, "Memory Full", 1013);
                        break;
                    case WS_REGISTER_ERR_FULL:
                        ws_close(req, "Server Full", 1013);
                        break;
                }

                return fd;
            }

            (void)ws_process(ws_rx, frame.len);

            return ESP_OK;
        case HTTPD_WS_TYPE_CLOSE:
            ws_unregister(req);
            return ESP_OK;
        default:
            return ESP_OK;
    }
}

#ifdef __cplusplus
}
#endif