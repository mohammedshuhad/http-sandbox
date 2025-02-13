#include "WifiBackend.hpp"

WifiBackend::WifiBackend() : 
s_semph_get_ip_addrs(nullptr), s_retry_num(0), s_sta_netif(nullptr), wifi_event_instance(nullptr), wifi_ip_instance(nullptr)
{
    ESP_ERROR_CHECK(nvs_flash_init()); // TODO : Move to hwinit
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(wifi_connect());
}

esp_err_t WifiBackend::wifi_connect(void)
{
    wifi_start();
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = wifi_ssid,
            .password = wifi_password,
            .scan_method = wifi_scan_method,
            .sort_method = wifi_connect_ap_sort_method,
            .threshold = {
                .rssi = wifi_scan_rssi_threshold,
                .authmode = wifi_scan_auth_mode_threshold}}};

    return wifi_sta_do_connect(wifi_config, true);
}

void WifiBackend::event_handler_got_ip(esp_event_base_t event_base, ip_event_got_ip_t *event_data)
{
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    ESP_LOGI(TAG, "Got IPv4 event: Interface \"%s\" address: " IPSTR, esp_netif_get_desc(event->esp_netif), IP2STR(&event->ip_info.ip));
    if (s_semph_get_ip_addrs)
    {
        xSemaphoreGive(s_semph_get_ip_addrs);
    }
    else
    {
        ESP_LOGI(TAG, "- IPv4 address: " IPSTR ",", IP2STR(&event->ip_info.ip));
    }
}

esp_err_t WifiBackend::wifi_sta_do_disconnect(void)
{
    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        WIFI_EVENT,
        WIFI_EVENT_STA_DISCONNECTED,
        wifi_event_instance
    ));

    ESP_ERROR_CHECK(esp_event_handler_instance_unregister(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        wifi_ip_instance
    ));
    
    if (s_semph_get_ip_addrs)
    {
        vSemaphoreDelete(s_semph_get_ip_addrs);
    }
    return esp_wifi_disconnect();
}

void WifiBackend::event_handler_on_wifi_disconnect(esp_event_base_t event_base, wifi_event_sta_disconnected_t *event_data)
{
    s_retry_num++;
    if (s_retry_num > wifi_conn_max_retry)
    {
        ESP_LOGI(TAG, "WiFi Connect failed %d times, stop reconnect.", s_retry_num);
        if (s_semph_get_ip_addrs)
        {
            xSemaphoreGive(s_semph_get_ip_addrs);
        }
        wifi_sta_do_disconnect();
        return;
    }
}

void WifiBackend::event_handler_c_wrp(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    WifiBackend *me = (WifiBackend *)arg;
    if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            me->event_handler_got_ip(event_base, (ip_event_got_ip_t *)event_data);
        }
        else
        {
            // Does Nothing
        }
    }
    else if (event_base == WIFI_EVENT)
    {
        if(event_id == WIFI_EVENT_STA_DISCONNECTED)
        {
            me->event_handler_on_wifi_disconnect(event_base, (wifi_event_sta_disconnected_t *)event_data);
        }
        else
        {
            // Does Nothing
        }
    }
    else
    {
        // Does Nothing
    }
}

void WifiBackend::wifi_start(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    esp_netif_config.if_desc = netif_desc_sta;
    esp_netif_config.route_prio = 128;
    s_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t WifiBackend::wifi_sta_do_connect(wifi_config_t wifi_config, bool wait)
{
    if (wait)
    {
        s_semph_get_ip_addrs = xSemaphoreCreateBinary();
        if (s_semph_get_ip_addrs == NULL)
        {
            return ESP_ERR_NO_MEM;
        }
    }
    s_retry_num = 0;

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        WIFI_EVENT_STA_DISCONNECTED,
        event_handler_c_wrp, this, wifi_event_instance
    ));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        event_handler_c_wrp, this, wifi_ip_instance
    ));
    
    ESP_LOGI(TAG, "Connecting to %s...", wifi_config.sta.ssid);
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config));
    esp_err_t ret = esp_wifi_connect();
    if (ret != ESP_OK)
    {
        ESP_LOGE(TAG, "WiFi connect failed! ret:%x", ret);
        return ret;
    }
    if (wait)
    {
        ESP_LOGI(TAG, "Waiting for IP(s)");
        xSemaphoreTake(s_semph_get_ip_addrs, portMAX_DELAY);
        if (s_retry_num > wifi_conn_max_retry)
        {
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}


