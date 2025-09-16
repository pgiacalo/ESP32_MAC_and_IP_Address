#ifndef ESP32_MAC_IP_H
#define ESP32_MAC_IP_H

#include <stdint.h>
#include <stdbool.h>
#include "esp_err.h"
#include "esp_netif.h"
#include "esp_mac.h"

#ifdef __cplusplus
extern "C" {
#endif

// Configuration structure for WiFi connection
typedef struct {
    char ssid[32];
    char password[64];
    uint8_t max_retry;
} esp32_mac_ip_wifi_config_t;

// Structure to hold MAC address information
typedef struct {
    uint8_t mac[6];
    char mac_string[18]; // "XX:XX:XX:XX:XX:XX" format
} mac_info_t;

// Structure to hold IP address information
typedef struct {
    esp_ip4_addr_t ip;
    esp_ip4_addr_t netmask;
    esp_ip4_addr_t gateway;
    char ip_string[16]; // "XXX.XXX.XXX.XXX" format
    char netmask_string[16];
    char gateway_string[16];
} ip_info_t;

// Structure to hold complete network information
typedef struct {
    mac_info_t mac;
    ip_info_t ip;
    bool wifi_connected;
    char ssid[32];
} network_info_t;

/**
 * @brief Initialize the MAC and IP library
 * 
 * This function initializes NVS flash storage which is required for WiFi operations.
 * Must be called before any other library functions.
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t esp32_mac_ip_init(void);

/**
 * @brief Get the MAC address of the ESP32
 * 
 * @param mac_type The type of MAC address to retrieve (ESP_MAC_WIFI_STA, ESP_MAC_WIFI_SOFTAP, etc.)
 * @param mac_info Pointer to mac_info_t structure to store the MAC address information
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t esp32_mac_ip_get_mac(esp_mac_type_t mac_type, mac_info_t *mac_info);

/**
 * @brief Connect to WiFi network and get IP address
 * 
 * This function connects to the specified WiFi network and retrieves the assigned IP address.
 * The function will block until connection is established or fails after maximum retries.
 * 
 * @param wifi_config Pointer to wifi_config_t structure containing SSID, password, and retry count
 * @param ip_info Pointer to ip_info_t structure to store the IP address information
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t esp32_mac_ip_connect_wifi(const esp32_mac_ip_wifi_config_t *wifi_config, ip_info_t *ip_info);

/**
 * @brief Get complete network information (MAC + IP)
 * 
 * This is a convenience function that gets both MAC address and IP address information
 * in a single call. WiFi connection is established if not already connected.
 * 
 * @param wifi_config Pointer to wifi_config_t structure for WiFi connection
 * @param mac_type The type of MAC address to retrieve
 * @param network_info Pointer to network_info_t structure to store all network information
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t esp32_mac_ip_get_network_info(const esp32_mac_ip_wifi_config_t *wifi_config, 
                                       esp_mac_type_t mac_type, 
                                       network_info_t *network_info);

/**
 * @brief Print MAC address information to console
 * 
 * @param mac_info Pointer to mac_info_t structure containing MAC address information
 */
void esp32_mac_ip_print_mac(const mac_info_t *mac_info);

/**
 * @brief Print IP address information to console
 * 
 * @param ip_info Pointer to ip_info_t structure containing IP address information
 * @param ssid WiFi SSID name (optional, can be NULL)
 */
void esp32_mac_ip_print_ip(const ip_info_t *ip_info, const char *ssid);

/**
 * @brief Print complete network information to console
 * 
 * @param network_info Pointer to network_info_t structure containing all network information
 */
void esp32_mac_ip_print_network_info(const network_info_t *network_info);

/**
 * @brief Check if WiFi is currently connected
 * 
 * @return true if WiFi is connected, false otherwise
 */
bool esp32_mac_ip_is_wifi_connected(void);

/**
 * @brief Disconnect from WiFi network
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t esp32_mac_ip_disconnect_wifi(void);

/**
 * @brief Deinitialize the MAC and IP library
 * 
 * This function cleans up resources and disconnects from WiFi if connected.
 * Should be called when the library is no longer needed.
 * 
 * @return ESP_OK on success, error code otherwise
 */
esp_err_t esp32_mac_ip_deinit(void);

#ifdef __cplusplus
}
#endif

#endif // ESP32_MAC_IP_H
