#include <stdio.h>
#include <string.h>
#include "esp32_mac_ip.h"

// WiFi configuration
#define WIFI_SSID      "YOUR_WIFI_SSID"
#define WIFI_PASS      "YOUR_WIFI_PASSWORD"
#define WIFI_MAXIMUM_RETRY  5



void app_main(void)
{
    // Initialize the MAC and IP library
    esp_err_t ret = esp32_mac_ip_init();
    if (ret != ESP_OK) {
        printf("Failed to initialize MAC and IP library: %s\n", esp_err_to_name(ret));
        return;
    }

    // Configure WiFi connection parameters
    esp32_mac_ip_wifi_config_t wifi_config = {
        .ssid = WIFI_SSID,
        .password = WIFI_PASS,
        .max_retry = WIFI_MAXIMUM_RETRY
    };

    // Get complete network information (MAC + IP)
    network_info_t network_info;
    ret = esp32_mac_ip_get_network_info(&wifi_config, ESP_MAC_WIFI_STA, &network_info);
    if (ret != ESP_OK) {
        printf("Failed to get network information: %s\n", esp_err_to_name(ret));
        return;
    }

    // Print all network information
    esp32_mac_ip_print_network_info(&network_info);

    // Optional: Clean up resources when done
    // esp32_mac_ip_deinit();
}
