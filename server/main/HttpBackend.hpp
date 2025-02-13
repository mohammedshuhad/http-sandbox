#pragma once

#include <esp_http_server.h>
#include "cJSON.h"

#define NGX_ESCAPE_URI (0)
#define NGX_ESCAPE_ARGS (1)
#define NGX_ESCAPE_URI_COMPONENT (2)
#define NGX_ESCAPE_HTML (3)
#define NGX_ESCAPE_REFRESH (4)
#define NGX_ESCAPE_MEMCACHED (5)
#define NGX_ESCAPE_MAIL_AUTH (6)
#define NGX_UNESCAPE_URI (1)
#define NGX_UNESCAPE_REDIRECT (2)

class HttpBackend
{
public:
    HttpBackend();

private:
    static constexpr const char *TAG = "HttpBackend";
    static constexpr int http_query_key_max_len = 64;

    httpd_handle_t server;
    int value[10];

    httpd_handle_t start_webserver(void);
    esp_err_t stop_webserver(httpd_handle_t server);
    
    void connect_handler(void *arg);
    void disconnect_handler(void *arg);
    static void event_handler_c_wrp(void *arg, esp_event_base_t event_base, int32_t event_id, void* event_data);

    httpd_uri_t get;
    httpd_uri_t set;
    static esp_err_t get_handler(httpd_req_t *req);
    static esp_err_t set_handler(httpd_req_t *req);
    
    void uri_decode(char *dest, const char *src, size_t len);
    void ngx_unescape_uri(u_char **dst, u_char **src, size_t size, unsigned int type);
};
