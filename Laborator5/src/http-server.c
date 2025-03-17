#include <string.h>
#include <sys/param.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"

#include "lwip/err.h"
#include "lwip/sys.h"
#include "freertos/event_groups.h"

#include "esp_http_server.h"

#define MAX_SSID_LIST 10  // Am mărit numărul de rețele pentru a permite mai multe rețele
#define MAX_SSID_LENGTH 32

// Structură pentru a stoca SSID și signal strength
typedef struct {
    char ssid[MAX_SSID_LENGTH];
    int signal_strength;
} wifi_network_t;

// Global Wi-Fi scan results
wifi_network_t wifi_networks[MAX_SSID_LIST];
int num_wifi_networks = 0;

// Functie pentru a scana rețelele Wi-Fi disponibile
esp_err_t scan_wifi_networks()
{
    wifi_ap_record_t ap_info[MAX_SSID_LIST];
    uint16_t ap_count = 0;

    // Inițializăm scanner-ul Wi-Fi
    esp_wifi_scan_start(NULL, true);
    vTaskDelay(5000 / portTICK_PERIOD_MS);  // Așteptăm pentru 5 secunde pentru a obține rețelele

    esp_wifi_scan_get_ap_records(&ap_count, ap_info);

    // Salvăm rețelele găsite într-o listă
    num_wifi_networks = ap_count;
    for (int i = 0; i < ap_count; i++) {
        strncpy(wifi_networks[i].ssid, (char*)ap_info[i].ssid, MAX_SSID_LENGTH);
        wifi_networks[i].signal_strength = ap_info[i].rssi;
    }

    return ESP_OK;
}

// Functie pentru a conecta ESP32 la rețeaua Wi-Fi
esp_err_t connect_to_wifi(const char *ssid, const char *password)
{
    esp_wifi_disconnect();  // Ne deconectăm de la orice rețea existentă

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = "",
            .password = ""
        },
    };

    // Setăm SSID-ul și parola în configurația Wi-Fi
    strncpy((char*)wifi_config.sta.ssid, ssid, sizeof(wifi_config.sta.ssid));
    strncpy((char*)wifi_config.sta.password, password, sizeof(wifi_config.sta.password));

    // Setăm Wi-Fi în modul station (sta)
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    esp_wifi_start();

    // Încercăm să ne conectăm la rețea
    esp_err_t err = esp_wifi_connect();
    return err;
}

// Handler pentru GET la /index.html
esp_err_t get_handler(httpd_req_t *req)
{
    // Scanează rețelele Wi-Fi disponibile
    scan_wifi_networks();

    char resp[1024];
    int len = 0;

    // Formulăm răspunsul HTML
    len += snprintf(resp + len, sizeof(resp) - len, "<html><body>");
    len += snprintf(resp + len, sizeof(resp) - len, "<form action='/results.html' method='post'>");
    len += snprintf(resp + len, sizeof(resp) - len, "<label for='fname'>Networks found:</label><br>");
    len += snprintf(resp + len, sizeof(resp) - len, "<select name='ssid'>");

    // Adăugăm rețelele Wi-Fi găsite în dropdown
    for (int i = 0; i < num_wifi_networks; i++) {
        len += snprintf(resp + len, sizeof(resp) - len, "<option value='%s'>%s</option>", wifi_networks[i].ssid, wifi_networks[i].ssid);
    }

    len += snprintf(resp + len, sizeof(resp) - len, "</select><br>");
    len += snprintf(resp + len, sizeof(resp) - len, "<label for='ipass'>Security key:</label><br>");
    len += snprintf(resp + len, sizeof(resp) - len, "<input type='password' name='ipass'><br>");
    len += snprintf(resp + len, sizeof(resp) - len, "<input type='submit' value='Submit'>");
    len += snprintf(resp + len, sizeof(resp) - len, "</form></body></html>");

    // Trimiterea răspunsului
    httpd_resp_send(req, resp, len);

    return ESP_OK;
}


// Handler pentru POST la /results.html
esp_err_t post_handler(httpd_req_t *req)
{
    char content[100];
    size_t recv_size = MIN(req->content_len, sizeof(content));

    int ret = httpd_req_recv(req, content, recv_size);
    if (ret <= 0) {
        if (ret == HTTPD_SOCK_ERR_TIMEOUT) {
            httpd_resp_send_408(req);
        }
        return ESP_FAIL;
    }

    // Pregătirea răspunsului
    const char *resp_header = "<html><body><h2>Results</h2>";
    const char *resp_footer = "<br><a href='/index.html'>Back to form</a></body></html>";

    char ssid[32] = {0};
    char password[32] = {0};
    httpd_query_key_value(content, "ssid", ssid, sizeof(ssid));
    httpd_query_key_value(content, "ipass", password, sizeof(password));

    // Conectăm ESP32 la rețeaua Wi-Fi
    esp_err_t err = connect_to_wifi(ssid, password);
    char resp[1024];
    int len = 0;

    if (err == ESP_OK) {
        ESP_LOGI("Wi-Fi", "Conectare la %s reușită!", ssid);
        len += snprintf(resp + len, sizeof(resp) - len, "%s", resp_header);
        len += snprintf(resp + len, sizeof(resp) - len, "<p>Conectare la rețeaua %s reușită!</p>", ssid);
    } else {
        ESP_LOGE("Wi-Fi", "Eroare la conectarea la rețeaua Wi-Fi %s", ssid);
        len += snprintf(resp + len, sizeof(resp) - len, "%s", resp_header);
        len += snprintf(resp + len, sizeof(resp) - len, "<p>Eroare la conectarea la rețeaua %s.</p>", ssid);
    }

    len += snprintf(resp + len, sizeof(resp) - len, "%s", resp_footer);

    // Trimiterea răspunsului
    httpd_resp_send(req, resp, len);

    return ESP_OK;
}


// Handler-uri pentru URI-urile GET și POST
httpd_uri_t uri_get = {
    .uri      = "/index.html",
    .method   = HTTP_GET,
    .handler  = get_handler,
    .user_ctx = NULL
};

httpd_uri_t uri_post = {
    .uri      = "/results.html",
    .method   = HTTP_POST,
    .handler  = post_handler,
    .user_ctx = NULL
};

// Funcția pentru pornirea serverului HTTP
httpd_handle_t start_webserver(void)
{
    httpd_config_t config = HTTPD_DEFAULT_CONFIG();
    httpd_handle_t server = NULL;

    if (httpd_start(&server, &config) == ESP_OK) {
        httpd_register_uri_handler(server, &uri_get);
        httpd_register_uri_handler(server, &uri_post);
    }
    return server;
}

// Funcția pentru oprirea serverului HTTP
void stop_webserver(httpd_handle_t server)
{
    if (server) {
        httpd_stop(server);
    }
}
