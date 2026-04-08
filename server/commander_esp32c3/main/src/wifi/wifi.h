#pragma once

#include "esp_mac.h"
#include "esp_netif.h"
#include "esp_wifi.h"
#include "lwip/ip_addr.h"
#include "wifi_ap.h"
#include "wifi_sta.h"

#ifdef __cplusplus
extern "C" {
#endif

typedef struct __attribute__((packed)) {
    uint8_t mac[6];
    esp_ip4_addr_t ip;
    const char* hostname;
} wifi_ap_client_t;

static wifi_ap_client_t wifi_ap_clients[CONFIG_ESP_MAX_STA_CONN_AP];
static uint32_t wifi_ap_client_count = 0;

#define DHCPS_OFFER_DNS 0x02

static void wifi_ap_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    switch (event_id) {
        case WIFI_EVENT_AP_STACONNECTED: {
            const wifi_event_ap_staconnected_t* event = event_data;
            ESP_LOGI(TAG_AP, "Station " MACSTR " joined, AID=%d", MAC2STR(event->mac), event->aid);

            wifi_ap_client_t client = {0};
            memcpy(client.mac, event->mac, sizeof(client.mac));
            wifi_ap_clients[wifi_ap_client_count++] = client;
            break;
        }

        case WIFI_EVENT_AP_STADISCONNECTED: {
            const wifi_event_ap_stadisconnected_t* event = event_data;
            ESP_LOGI(TAG_AP, "Station " MACSTR " left, AID=%d, reason:%d", MAC2STR(event->mac), event->aid, event->reason);
            wifi_ap_client_count--;
            break;
        }
    }
}

static void wifi_sta_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    if (event_id == WIFI_EVENT_STA_START) {
        ESP_LOGI(TAG_STA, "Station started");
    } else if (event_id == WIFI_EVENT_STA_CONNECTED) {
        // const wifi_event_sta_connected_t* event = event_data;
        ESP_LOGI(TAG_STA, "WiFi connected");
    } else if (event_id == WIFI_EVENT_STA_DISCONNECTED) {
        const wifi_event_sta_disconnected_t* event = event_data;
        ESP_LOGI(TAG_STA, "WiFi disconnected, reason=%d", event->reason);
    }
}

static void ip_event_handler(void* arg, esp_event_base_t event_base, int32_t event_id, void* event_data) {
    switch (event_id) {
        case IP_EVENT_STA_GOT_IP:
            const ip_event_got_ip_t* event = (ip_event_got_ip_t*)event_data;
            ESP_LOGI(TAG_STA, "Got IP:" IPSTR, IP2STR(&event->ip_info.ip));
            break;
        case IP_EVENT_ASSIGNED_IP_TO_CLIENT:
            const ip_event_assigned_ip_to_client_t* e = event_data;
            ESP_LOGI(TAG_AP, "Assigned IP to client: " IPSTR ", MAC=" MACSTR ", hostname='%s'", IP2STR(&e->ip), MAC2STR(e->mac), e->hostname);
            for (size_t i = 0; i < wifi_ap_client_count; i++) {
                if (memcmp(wifi_ap_clients[i].mac, e->mac, sizeof(wifi_ap_clients[i].mac)) == 0) {
                    wifi_ap_clients[i].ip = e->ip;
                    wifi_ap_clients[i].hostname = strdup(e->hostname);
                    ;
                }
            }

            break;
    }
}

static void wifi_init(void) {
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STACONNECTED, &wifi_ap_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_AP_STADISCONNECTED, &wifi_ap_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_START, &wifi_sta_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &wifi_sta_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &wifi_sta_event_handler, NULL, NULL));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &ip_event_handler, NULL, NULL));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT, IP_EVENT_ASSIGNED_IP_TO_CLIENT, &ip_event_handler, NULL, NULL));

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_APSTA));
    esp_netif_t* esp_netif_ap = wifi_init_softap();
    esp_netif_create_default_wifi_sta();
    wifi_config_t wifi_sta_config = {
        .sta = {
            .ssid = CONFIG_ESP_WIFI_REMOTE_AP_SSID,
            .password = CONFIG_ESP_WIFI_REMOTE_AP_PASSWORD,
            .threshold.authmode = ESP_WIFI_SCAN_AUTH_MODE_THRESHOLD,
            .sae_pwe_h2e = WPA3_SAE_PWE_BOTH,
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_sta_config));

    esp_netif_dns_info_t dns;
    dns.ip.type = ESP_IPADDR_TYPE_V4;
    dns.ip.u_addr.ip4.addr = ESP_IP4TOADDR(8, 8, 8, 8);

    uint8_t dhcps_offer_option = DHCPS_OFFER_DNS;
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_stop(esp_netif_ap));
    ESP_ERROR_CHECK(esp_netif_dhcps_option(esp_netif_ap, ESP_NETIF_OP_SET, ESP_NETIF_DOMAIN_NAME_SERVER, &dhcps_offer_option, sizeof(dhcps_offer_option)));
    ESP_ERROR_CHECK(esp_netif_set_dns_info(esp_netif_ap, ESP_NETIF_DNS_MAIN, &dns));
    ESP_ERROR_CHECK_WITHOUT_ABORT(esp_netif_dhcps_start(esp_netif_ap));

    ESP_ERROR_CHECK(esp_wifi_start());

    esp_wifi_set_max_tx_power(34);

    if (esp_netif_napt_enable(esp_netif_ap) != ESP_OK)
        ESP_LOGE(TAG_STA, "NAPT not enabled on netif: %p", esp_netif_ap);
}

#ifdef __cplusplus
}
#endif