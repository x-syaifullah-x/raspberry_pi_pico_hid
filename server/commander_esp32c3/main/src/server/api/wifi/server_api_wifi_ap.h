#pragma once

#include "cJSON.h"
#include "esp_http_server.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "server_api_wifi_ap_uri.h"
#include "wifi/wifi.h"

#ifdef __cplusplus
extern "C" {
#endif

static esp_err_t server_api_wifi_ap_handler(httpd_req_t* req) {
    if (strcmp(req->uri, server_api_wifi_ap_uri_status.uri) == 0) {
        esp_netif_t* ap_netif = esp_netif_get_handle_from_ifkey("WIFI_AP_DEF");
        if (!ap_netif) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "AP netif not found");
            return ESP_FAIL;
        }

        esp_netif_ip_info_t esp_netif_ip_info;
        esp_err_t err = esp_netif_get_ip_info(ap_netif, &esp_netif_ip_info);
        if (err != ESP_OK) {
            httpd_resp_send_err(req, HTTPD_500_INTERNAL_SERVER_ERROR, "Failed to get AP IP");
            return ESP_FAIL;
        }

        cJSON* status = cJSON_CreateObject();

        char ip[16], netmask[16], gw[16];

        esp_ip4addr_ntoa(&esp_netif_ip_info.ip, ip, sizeof(ip));
        esp_ip4addr_ntoa(&esp_netif_ip_info.netmask, netmask, sizeof(netmask));
        esp_ip4addr_ntoa(&esp_netif_ip_info.gw, gw, sizeof(gw));

        cJSON_AddStringToObject(status, "ip", ip);
        cJSON_AddStringToObject(status, "netmask", netmask);
        cJSON_AddStringToObject(status, "gateway", gw);

        cJSON* clients = cJSON_AddArrayToObject(status, "clients");
        for (size_t i = 0; i < wifi_ap_client_count; i++) {
            const wifi_ap_client_t *ap_client = &wifi_ap_clients[i];
            cJSON* client = cJSON_CreateObject();
            char mac[18];
            snprintf(mac, sizeof(mac),
                     "%02X:%02X:%02X:%02X:%02X:%02X",
                     ap_client->mac[0], ap_client->mac[1], ap_client->mac[2], ap_client->mac[3], ap_client->mac[4], ap_client->mac[5]);
            cJSON_AddStringToObject(client, "mac", mac);

            esp_ip4_addr_t ap_client_ip = ap_client->ip;
            char client_ip[16];
            esp_ip4addr_ntoa(&ap_client_ip, client_ip, sizeof(client_ip));
            cJSON_AddStringToObject(client, "ip", client_ip);

            if (ap_client->hostname) {
                cJSON_AddStringToObject(client, "hostname", ap_client->hostname);
            } else {
                cJSON_AddNullToObject(client, "hostname");
            }

            cJSON_AddItemToArray(clients, client);
        }

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