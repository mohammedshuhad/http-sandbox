#pragma once
#include <esp_log.h>
#include <nvs_flash.h>
#include "esp_netif.h"
#include "esp_event.h"
#include "esp_check.h"
#include "esp_wifi.h"
#include "cJSON.h"
#include <string>

#define wifi_ssid "KITEZ INCUBATION"
#define wifi_password "Kitez123!"
#define wifi_scan_method WIFI_ALL_CHANNEL_SCAN
#define wifi_connect_ap_sort_method WIFI_CONNECT_AP_BY_SIGNAL
#define wifi_scan_rssi_threshold -127
#define wifi_scan_auth_mode_threshold WIFI_AUTH_OPEN

class WifiBackend
{
public:
    WifiBackend();

private:
    static constexpr const char *TAG = "WifiBackend";
    static constexpr int wifi_conn_max_retry = 6;
    static constexpr const char *netif_desc_sta = "pulsator_netif_sta";

    void wifi_start(void);
    esp_err_t wifi_connect(void);
    esp_err_t wifi_sta_do_connect(wifi_config_t wifi_config, bool wait);
    esp_err_t wifi_sta_do_disconnect(void);
    void event_handler_got_ip(esp_event_base_t event_base, ip_event_got_ip_t *event_data);
    void event_handler_on_wifi_disconnect(esp_event_base_t event_base, wifi_event_sta_disconnected_t *event_data);

    static void event_handler_c_wrp(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    SemaphoreHandle_t s_semph_get_ip_addrs;
    int s_retry_num;
    esp_netif_t *s_sta_netif;
    esp_event_handler_instance_t *wifi_event_instance;
    esp_event_handler_instance_t *wifi_ip_instance;
};