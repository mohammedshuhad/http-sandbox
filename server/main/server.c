#include <stdio.h>
#include <esp_log.h>
#include <nvs_flash.h>
#include "esp_netif.h"
#include <esp_http_server.h>
#include "esp_event.h"
#include "esp_check.h"
#include "esp_wifi.h"

static const char *TAG = "Server";

#define CONFIG_EXAMPLE_WIFI_SSID "KITEZ INCUBATION"
#define CONFIG_EXAMPLE_WIFI_PASSWORD "Kitez123!"
#define EXAMPLE_WIFI_SCAN_METHOD WIFI_ALL_CHANNEL_SCAN
#define EXAMPLE_WIFI_CONNECT_AP_SORT_METHOD WIFI_CONNECT_AP_BY_SIGNAL
#define CONFIG_EXAMPLE_WIFI_SCAN_RSSI_THRESHOLD -127
#define EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD WIFI_AUTH_OPEN
#define CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY 6
#define EXAMPLE_NETIF_DESC_STA "denada_netif_sta"
#define EXAMPLE_HTTP_QUERY_KEY_MAX_LEN (64)

#define NGX_ESCAPE_URI (0)
#define NGX_ESCAPE_ARGS (1)
#define NGX_ESCAPE_URI_COMPONENT (2)
#define NGX_ESCAPE_HTML (3)
#define NGX_ESCAPE_REFRESH (4)
#define NGX_ESCAPE_MEMCACHED (5)
#define NGX_ESCAPE_MAIL_AUTH (6)

/* Type of Unescape algorithms to be used */
#define NGX_UNESCAPE_URI (1)
#define NGX_UNESCAPE_REDIRECT (2)

static esp_netif_t *s_example_sta_netif = NULL;
static SemaphoreHandle_t s_semph_get_ip_addrs = NULL;
static int s_retry_num = 0;

static void example_handler_on_sta_got_ip(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void example_handler_on_wifi_disconnect(void *arg, esp_event_base_t event_base, int32_t event_id, void *event_data);
static void example_handler_on_wifi_connect(void *esp_netif, esp_event_base_t event_base, int32_t event_id, void *event_data);

static esp_err_t stop_webserver(httpd_handle_t server);

esp_err_t
example_wifi_sta_do_disconnect(void)
{
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &example_handler_on_wifi_disconnect));
    ESP_ERROR_CHECK(esp_event_handler_unregister(IP_EVENT, IP_EVENT_STA_GOT_IP, &example_handler_on_sta_got_ip));
    ESP_ERROR_CHECK(esp_event_handler_unregister(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &example_handler_on_wifi_connect));
    if (s_semph_get_ip_addrs)
    {
        vSemaphoreDelete(s_semph_get_ip_addrs);
    }
    return esp_wifi_disconnect();
}

static void example_handler_on_wifi_disconnect(void *arg, esp_event_base_t event_base,
                                               int32_t event_id, void *event_data)
{
    s_retry_num++;
    if (s_retry_num > CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY)
    {
        ESP_LOGI(TAG, "WiFi Connect failed %d times, stop reconnect.", s_retry_num);
        /* let example_wifi_sta_do_connect() return */
        if (s_semph_get_ip_addrs)
        {
            xSemaphoreGive(s_semph_get_ip_addrs);
        }
        example_wifi_sta_do_disconnect();
        return;
    }
    wifi_event_sta_disconnected_t *disconn = event_data;
    if (disconn->reason == WIFI_REASON_ROAMING)
    {
        ESP_LOGD(TAG, "station roaming, do nothing");
        return;
    }
    ESP_LOGI(TAG, "Wi-Fi disconnected %d, trying to reconnect...", disconn->reason);
    esp_err_t err = esp_wifi_connect();
    if (err == ESP_ERR_WIFI_NOT_STARTED)
    {
        return;
    }
    ESP_ERROR_CHECK(err);
}

static void example_handler_on_wifi_connect(void *esp_netif, esp_event_base_t event_base,
                                            int32_t event_id, void *event_data)
{
}

static void example_handler_on_sta_got_ip(void *arg, esp_event_base_t event_base,
                                          int32_t event_id, void *event_data)
{
    s_retry_num = 0;
    ip_event_got_ip_t *event = (ip_event_got_ip_t *)event_data;
    // if (!example_is_our_netif(EXAMPLE_NETIF_DESC_STA, event->esp_netif))
    // {
    //     return;
    // }
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

esp_err_t example_wifi_sta_do_connect(wifi_config_t wifi_config, bool wait)
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
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &example_handler_on_wifi_disconnect, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &example_handler_on_sta_got_ip, NULL));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_CONNECTED, &example_handler_on_wifi_connect, s_example_sta_netif));

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
        if (s_retry_num > CONFIG_EXAMPLE_WIFI_CONN_MAX_RETRY)
        {
            return ESP_FAIL;
        }
    }
    return ESP_OK;
}

void example_wifi_start(void)
{
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_netif_inherent_config_t esp_netif_config = ESP_NETIF_INHERENT_DEFAULT_WIFI_STA();
    // Warning: the interface desc is used in tests to capture actual connection details (IP, gw, mask)
    esp_netif_config.if_desc = EXAMPLE_NETIF_DESC_STA;
    esp_netif_config.route_prio = 128;
    s_example_sta_netif = esp_netif_create_wifi(WIFI_IF_STA, &esp_netif_config);
    esp_wifi_set_default_wifi_sta_handlers();

    ESP_ERROR_CHECK(esp_wifi_set_storage(WIFI_STORAGE_RAM));
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA));
    ESP_ERROR_CHECK(esp_wifi_start());
}

esp_err_t example_wifi_connect(void)
{
    ESP_LOGI(TAG, "Start example_connect.");
    example_wifi_start();
    wifi_config_t wifi_config = {
        .sta = {
            .ssid = CONFIG_EXAMPLE_WIFI_SSID,
            .password = CONFIG_EXAMPLE_WIFI_PASSWORD,
            .scan_method = EXAMPLE_WIFI_SCAN_METHOD,
            .sort_method = EXAMPLE_WIFI_CONNECT_AP_SORT_METHOD,
            .threshold.rssi = CONFIG_EXAMPLE_WIFI_SCAN_RSSI_THRESHOLD,
            .threshold.authmode = EXAMPLE_WIFI_SCAN_AUTH_MODE_THRESHOLD,
        },
    };
    return example_wifi_sta_do_connect(wifi_config, true);
}

static void disconnect_handler(void *arg, esp_event_base_t event_base,
                               int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server)
    {
        ESP_LOGI(TAG, "Stopping webserver");
        if (stop_webserver(*server) == ESP_OK)
        {
            *server = NULL;
        }
        else
        {
            ESP_LOGE(TAG, "Failed to stop http server");
        }
    }
}

void ngx_unescape_uri(u_char **dst, u_char **src, size_t size, unsigned int type)
{
    u_char *d, *s, ch, c, decoded;
    enum
    {
        sw_usual = 0,
        sw_quoted,
        sw_quoted_second
    } state;

    d = *dst;
    s = *src;

    state = 0;
    decoded = 0;

    while (size--)
    {

        ch = *s++;

        switch (state)
        {
        case sw_usual:
            if (ch == '?' && (type & (NGX_UNESCAPE_URI | NGX_UNESCAPE_REDIRECT)))
            {
                *d++ = ch;
                goto done;
            }

            if (ch == '%')
            {
                state = sw_quoted;
                break;
            }

            *d++ = ch;
            break;

        case sw_quoted:

            if (ch >= '0' && ch <= '9')
            {
                decoded = (u_char)(ch - '0');
                state = sw_quoted_second;
                break;
            }

            c = (u_char)(ch | 0x20);
            if (c >= 'a' && c <= 'f')
            {
                decoded = (u_char)(c - 'a' + 10);
                state = sw_quoted_second;
                break;
            }

            /* the invalid quoted character */

            state = sw_usual;

            *d++ = ch;

            break;

        case sw_quoted_second:

            state = sw_usual;

            if (ch >= '0' && ch <= '9')
            {
                ch = (u_char)((decoded << 4) + (ch - '0'));

                if (type & NGX_UNESCAPE_REDIRECT)
                {
                    if (ch > '%' && ch < 0x7f)
                    {
                        *d++ = ch;
                        break;
                    }

                    *d++ = '%';
                    *d++ = *(s - 2);
                    *d++ = *(s - 1);

                    break;
                }

                *d++ = ch;

                break;
            }

            c = (u_char)(ch | 0x20);
            if (c >= 'a' && c <= 'f')
            {
                ch = (u_char)((decoded << 4) + (c - 'a') + 10);

                if (type & NGX_UNESCAPE_URI)
                {
                    if (ch == '?')
                    {
                        *d++ = ch;
                        goto done;
                    }

                    *d++ = ch;
                    break;
                }

                if (type & NGX_UNESCAPE_REDIRECT)
                {
                    if (ch == '?')
                    {
                        *d++ = ch;
                        goto done;
                    }

                    if (ch > '%' && ch < 0x7f)
                    {
                        *d++ = ch;
                        break;
                    }

                    *d++ = '%';
                    *d++ = *(s - 2);
                    *d++ = *(s - 1);
                    break;
                }

                *d++ = ch;

                break;
            }

            /* the invalid quoted character */

            break;
        }
    }

done:

    *dst = d;
    *src = s;
}

void example_uri_decode(char *dest, const char *src, size_t len)
{
    if (!src || !dest)
    {
        return;
    }

    unsigned char *src_ptr = (unsigned char *)src;
    unsigned char *dst_ptr = (unsigned char *)dest;
    ngx_unescape_uri(&dst_ptr, &src_ptr, len, NGX_UNESCAPE_URI);
}

static esp_err_t hello_get_handler(httpd_req_t *req)
{
    char *buf;
    size_t buf_len;

    /* Get header value string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
        /* Copy null terminated value string into buffer */
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
        if (httpd_req_get_hdr_value_str(req, "Test-Header-2", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Test-Header-2: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-1") + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    /* Read URL query string length and allocate memory for length + 1,
     * extra byte for null termination */
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = malloc(buf_len);
        ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN], dec_param[EXAMPLE_HTTP_QUERY_KEY_MAX_LEN] = {0};
            /* Get value of expected key from query string */
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
                example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
                if(strcmp(dec_param, "1") == 0)
                {
                    ESP_LOGI(TAG, "Value is 1");
                }
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
                example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
                example_uri_decode(dec_param, param, strnlen(param, EXAMPLE_HTTP_QUERY_KEY_MAX_LEN));
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
            }
        }
        free(buf);
    }

    /* Set some custom headers */
    httpd_resp_set_hdr(req, "Custom-Header-1", "Custom-Value-1");
    httpd_resp_set_hdr(req, "Custom-Header-2", "Custom-Value-2");

    /* Send response with custom headers and body set as the
     * string passed in user context*/
    const char *resp_str = (const char *)req->user_ctx;
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0)
    {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

static const httpd_uri_t hello = {
    .uri = "/hello",
    .method = HTTP_GET,
    .handler = hello_get_handler,
    /* Let's pass response string in user
     * context to demonstrate it's usage */
    .user_ctx = "Hello World!"};

static httpd_handle_t start_webserver(void)
{
    httpd_handle_t server = NULL;
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    config.lru_purge_enable = true;

    // Start the httpd server
    ESP_LOGI(TAG, "Starting server on port: '%d'", config.server_port);
    if (httpd_start(&server, &config) == ESP_OK)
    {
        // Set URI handlers
        ESP_LOGI(TAG, "Registering URI handlers");
        httpd_register_uri_handler(server, &hello);
        // httpd_register_uri_handler(server, &echo);
        // httpd_register_uri_handler(server, &ctrl);
        // httpd_register_uri_handler(server, &any);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

static esp_err_t stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}

static void connect_handler(void *arg, esp_event_base_t event_base,
                            int32_t event_id, void *event_data)
{
    httpd_handle_t *server = (httpd_handle_t *)arg;
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void app_main(void)
{
    static httpd_handle_t server = NULL;

    ESP_ERROR_CHECK(nvs_flash_init());
    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());

    ESP_ERROR_CHECK(example_wifi_connect());

    ESP_ERROR_CHECK(esp_event_handler_register(IP_EVENT, IP_EVENT_STA_GOT_IP, &connect_handler, &server));
    ESP_ERROR_CHECK(esp_event_handler_register(WIFI_EVENT, WIFI_EVENT_STA_DISCONNECTED, &disconnect_handler, &server));

    server = start_webserver();

    while (server)
    {
        vTaskDelay(5000 / portTICK_PERIOD_MS);
    }
}
