/*  WiFi softAP Example

   This example code is in the Public Domain (or CC0 licensed, at your option.)

   Unless required by applicable law or agreed to in writing, this
   software is distributed on an "AS IS" BASIS, WITHOUT WARRANTIES OR CONDITIONS OF ANY KIND, either express or implied.
*/
#include <string.h>
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "esp_system.h"
#include "esp_wifi.h"
#include "esp_event.h"
#include "esp_log.h"
#include "nvs_flash.h"
#include "freertos/event_groups.h"
#include "esp_http_server.h"


#include "lwip/err.h"
#include "lwip/sys.h"

#include "soft-ap.h"
#include "http-server.h"

#include "../mdns/include/mdns.h"

static const char *TAG = "wifi softAP";

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    // Inițializare Wi-Fi în modul station (STA)
    ESP_LOGI(TAG, "Setting WiFi to STA mode...");
    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));
    esp_wifi_set_mode(WIFI_MODE_STA);
    esp_wifi_start();

    // Scanare rețele Wi-Fi
    ESP_LOGI(TAG, "Scanning Wi-Fi networks...");
    scan_wifi_networks();

    // Inițializare mDNS pentru hostname `setup.local`
    ESP_LOGI(TAG, "Initializing mDNS...");
    mdns_init();
    mdns_hostname_set("setup");  // Setează hostname-ul la "setup"
    mdns_service_add("http", "_http", "_tcp", 80, NULL, 0);  // Adaugă serviciul mDNS pe portul 80

    // Inițializare SoftAP
    ESP_LOGI(TAG, "ESP_WIFI_MODE_AP");
    wifi_init_softap();

    // Pornire server web
    ESP_LOGI(TAG, "Starting HTTP Server");
    httpd_handle_t server = start_webserver();
    if (server == NULL) {
        ESP_LOGI(TAG, "Error starting webserver!");
    } else {
        ESP_LOGI(TAG, "Webserver started successfully!");
    }
}