#include "HttpBackend.hpp"

HttpBackend::HttpBackend() 
:   server(nullptr),
    get{
        "/get", 
        HTTP_GET,
        &HttpBackend::get_handler,
        this 
    },
    set{
        "/set",
        HTTP_PUT,
        &HttpBackend::set_handler,
        this
    }
{
    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        WIFI_EVENT,
        WIFI_EVENT_STA_DISCONNECTED,
        event_handler_c_wrp, this, NULL
    ));

    ESP_ERROR_CHECK(esp_event_handler_instance_register(
        IP_EVENT,
        IP_EVENT_STA_GOT_IP,
        event_handler_c_wrp, this, NULL
    ));

    server = start_webserver();
    value[0] = 10;
    value[1] = 20;
    value[2] = 30;
}

void HttpBackend::event_handler_c_wrp(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data)
{
    HttpBackend *me = static_cast<HttpBackend*>(arg);
    if (event_base == IP_EVENT)
    {
        if (event_id == IP_EVENT_STA_GOT_IP)
        {
            me->connect_handler(me->server);
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
            me->disconnect_handler(me->server);
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

void HttpBackend::connect_handler(void *arg)
{
    httpd_handle_t *server = (httpd_handle_t *)(arg);
    if (*server == NULL)
    {
        ESP_LOGI(TAG, "Starting webserver");
        *server = start_webserver();
    }
}

void HttpBackend::disconnect_handler(void *arg)
{
    httpd_handle_t *server = (httpd_handle_t *)(arg);
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

esp_err_t HttpBackend::get_handler(httpd_req_t *req)
{
    HttpBackend* instance = static_cast<HttpBackend*>(req->user_ctx);
    if (!instance) {
        ESP_LOGE(TAG, "Instance pointer is null");
        return ESP_FAIL;
    }

    char *buf;
    size_t buf_len;

    buf_len = httpd_req_get_hdr_value_len(req, "Host") + 1;
    if (buf_len > 1)
    {
        buf = (char *)malloc(buf_len);
        ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
        if (httpd_req_get_hdr_value_str(req, "Host", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Host: %s", buf);
        }
        free(buf);
    }

    buf_len = httpd_req_get_hdr_value_len(req, "Test-Header-2") + 1;
    if (buf_len > 1)
    {
        buf = (char *)malloc(buf_len);
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
        buf = (char *)malloc(buf_len);
        ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
        if (httpd_req_get_hdr_value_str(req, "Test-Header-1", buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found header => Test-Header-1: %s", buf);
        }
        free(buf);
    }

    std::string rtrStr;
    buf_len = httpd_req_get_url_query_len(req) + 1;
    if (buf_len > 1)
    {
        buf = (char *)malloc(buf_len);
        ESP_RETURN_ON_FALSE(buf, ESP_ERR_NO_MEM, TAG, "buffer alloc failed");
        if (httpd_req_get_url_query_str(req, buf, buf_len) == ESP_OK)
        {
            ESP_LOGI(TAG, "Found URL query => %s", buf);
            char param[http_query_key_max_len], dec_param[http_query_key_max_len] = {0};
            if (httpd_query_key_value(buf, "query1", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found URL query parameter => query1=%s", param);
                instance->uri_decode(dec_param, param, strnlen(param, http_query_key_max_len));
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
                if (std::string(dec_param) == "1")
                {
                    rtrStr = std::to_string(instance->value[0]);
                }
                else if (std::string(dec_param) == "2")
                {
                    rtrStr = std::to_string(instance->value[1]);
                }
                else if (std::string(dec_param) == "3")
                {
                    rtrStr = std::to_string(instance->value[2]);
                }
            }
            if (httpd_query_key_value(buf, "query3", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found URL query parameter => query3=%s", param);
                instance->uri_decode(dec_param, param, strnlen(param, http_query_key_max_len));
                ESP_LOGI(TAG, "Decoded query parameter => %s", dec_param);
            }
            if (httpd_query_key_value(buf, "query2", param, sizeof(param)) == ESP_OK)
            {
                ESP_LOGI(TAG, "Found URL query parameter => query2=%s", param);
                instance->uri_decode(dec_param, param, strnlen(param, http_query_key_max_len));
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
    // const char *resp_str = (const char *)req->user_ctx;
    const char *resp_str = rtrStr.c_str();
    httpd_resp_send(req, resp_str, HTTPD_RESP_USE_STRLEN);

    /* After sending the HTTP response the old HTTP request
     * headers are lost. Check if HTTP request headers can be read now. */
    if (httpd_req_get_hdr_value_len(req, "Host") == 0)
    {
        ESP_LOGI(TAG, "Request headers lost");
    }
    return ESP_OK;
}

esp_err_t HttpBackend::set_handler(httpd_req_t *req)
{
    HttpBackend* instance = static_cast<HttpBackend*>(req->user_ctx);
    if (!instance) {
        ESP_LOGE(TAG, "Instance pointer is null");
        return ESP_FAIL;
    }

    char *buf;
    int total_len = req->content_len;
    int cur_len = 0;

    // Allocate memory for the request content
    buf = (char *)malloc(total_len + 1);
    if (buf == NULL)
    {
        ESP_LOGE(TAG, "Failed to allocate memory for PUT data");
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }
    buf[total_len] = '\0';

    // Receive the request content
    while (cur_len < total_len)
    {
        int received = httpd_req_recv(req, buf + cur_len, total_len - cur_len);
        if (received <= 0)
        {
            // Timeout or error occurred
            free(buf);
            if (received == HTTPD_SOCK_ERR_TIMEOUT)
            {
                httpd_resp_send_408(req);
            }
            return ESP_FAIL;
        }
        cur_len += received;
    }

    ESP_LOGI(TAG, "Received PUT data: %s", buf);

    // Parse the received data as JSON without using cJSON
    // Process the received data
    // Here you can parse the JSON or process the data as needed
    if (buf[0] == '0')
    {
        instance->value[0] = 0; // Example: modify the global value array
        ESP_LOGI(TAG, "Set value[0] to 0");
    }
    else
    {
        instance->value[0] = 1; // Example: modify the global value array
        ESP_LOGI(TAG, "Set value[0] to 1");
    }

    cJSON *root = cJSON_Parse(buf);
    if (root == NULL)
    {
        const char *error_ptr = cJSON_GetErrorPtr();
        if (error_ptr != NULL)
        {
            ESP_LOGE(TAG, "Error parsing JSON: %s", error_ptr);
        }
        free(buf);
        httpd_resp_send_500(req);
        return ESP_FAIL;
    }

    // Get the "value" field from JSON
    cJSON *value_obj = cJSON_GetObjectItem(root, "value");
    if (cJSON_IsNumber(value_obj))
    {
        instance->value[0] = value_obj->valueint;
        ESP_LOGI(TAG, "Set value[0] to %d", instance->value[0]);
    }
    else
    {
        ESP_LOGE(TAG, "Invalid or missing 'value' field in JSON");
        cJSON_Delete(root);
        free(buf);
        httpd_resp_send_408(req);
        return ESP_FAIL;
    }

    // Clean up
    cJSON_Delete(root);

    // Clean up
    free(buf);

    // Send response
    httpd_resp_set_type(req, "application/json");
    char resp[64];
    snprintf(resp, sizeof(resp), "{\"status\":\"ok\",\"value\":%d}", instance->value[0]);
    httpd_resp_send(req, resp, HTTPD_RESP_USE_STRLEN);

    return ESP_OK;
}

httpd_handle_t HttpBackend::start_webserver(void)
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
        httpd_register_uri_handler(server, &get);
        httpd_register_uri_handler(server, &set);
        return server;
    }

    ESP_LOGI(TAG, "Error starting server!");
    return NULL;
}

void HttpBackend::ngx_unescape_uri(u_char **dst, u_char **src, size_t size, unsigned int type)
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

    state = (decltype(state))0;
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

void HttpBackend::uri_decode(char *dest, const char *src, size_t len)
{
    if (!src || !dest)
    {
        return;
    }

    unsigned char *src_ptr = (unsigned char *)src;
    unsigned char *dst_ptr = (unsigned char *)dest;
    ngx_unescape_uri(&dst_ptr, &src_ptr, len, NGX_UNESCAPE_URI);
}

esp_err_t HttpBackend::stop_webserver(httpd_handle_t server)
{
    // Stop the httpd server
    return httpd_stop(server);
}
