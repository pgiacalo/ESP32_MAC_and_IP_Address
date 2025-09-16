#include "esp32_mac_ip.h"
#include <stdio.h>
#include <string.h>
#include "esp_system.h"
#include "esp_mac.h"
#include "esp_wifi.h"
#include "esp_netif.h"
#include "esp_event.h"
#include "nvs_flash.h"
#include "freertos/FreeRTOS.h"
#include "freertos/task.h"
#include "freertos/event_groups.h"

// Internal WiFi event group and bits
static EventGroupHandle_t s_wifi_event_group = NULL;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

// Internal retry counter
static int s_retry_num = 0;

// Internal WiFi configuration
static esp32_mac_ip_wifi_config_t s_current_wifi_config = {0};
static bool s_wifi_initialized = false;

/**
 * @brief Internal event handler for WiFi events
 */
static void wifi_event_handler(void* arg, esp_event_base_t event_base,
                              int32_t event_id, void* event_data)
{
    if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_START) {
        esp_wifi_connect();
    } else if (event_base == WIFI_EVENT && event_id == WIFI_EVENT_STA_DISCONNECTED) {
        wifi_event_sta_disconnected_t* disconnected = (wifi_event_sta_disconnected_t*) event_data;
        printf("WiFi disconnected. Reason: %d - ", disconnected->reason);
        
        // Provide human-readable error messages for common disconnect reasons
        switch (disconnected->reason) {
            case WIFI_REASON_UNSPECIFIED:
                printf("Unspecified reason");
                break;
            case WIFI_REASON_AUTH_EXPIRE:
                printf("Authentication expired");
                break;
            case WIFI_REASON_AUTH_LEAVE:
                printf("Authentication left");
                break;
            case WIFI_REASON_ASSOC_EXPIRE:
                printf("Association expired");
                break;
            case WIFI_REASON_ASSOC_TOOMANY:
                printf("Too many associations");
                break;
            case WIFI_REASON_NOT_AUTHED:
                printf("Not authenticated");
                break;
            case WIFI_REASON_NOT_ASSOCED:
                printf("Not associated");
                break;
            case WIFI_REASON_ASSOC_LEAVE:
                printf("Association left");
                break;
            case WIFI_REASON_ASSOC_NOT_AUTHED:
                printf("Association not authenticated");
                break;
            case WIFI_REASON_DISASSOC_PWRCAP_BAD:
                printf("Disassociation due to power capability");
                break;
            case WIFI_REASON_DISASSOC_SUPCHAN_BAD:
                printf("Disassociation due to supported channels");
                break;
            case WIFI_REASON_IE_INVALID:
                printf("Invalid information element");
                break;
            case WIFI_REASON_MIC_FAILURE:
                printf("MIC failure");
                break;
            case WIFI_REASON_4WAY_HANDSHAKE_TIMEOUT:
                printf("4-way handshake timeout");
                break;
            case WIFI_REASON_GROUP_KEY_UPDATE_TIMEOUT:
                printf("Group key update timeout");
                break;
            case WIFI_REASON_IE_IN_4WAY_DIFFERS:
                printf("Information element in 4-way handshake differs");
                break;
            case WIFI_REASON_GROUP_CIPHER_INVALID:
                printf("Group cipher invalid");
                break;
            case WIFI_REASON_PAIRWISE_CIPHER_INVALID:
                printf("Pairwise cipher invalid");
                break;
            case WIFI_REASON_AKMP_INVALID:
                printf("AKMP invalid");
                break;
            case WIFI_REASON_UNSUPP_RSN_IE_VERSION:
                printf("Unsupported RSN IE version");
                break;
            case WIFI_REASON_INVALID_RSN_IE_CAP:
                printf("Invalid RSN IE capabilities");
                break;
            case WIFI_REASON_802_1X_AUTH_FAILED:
                printf("802.1X authentication failed");
                break;
            case WIFI_REASON_CIPHER_SUITE_REJECTED:
                printf("Cipher suite rejected");
                break;
            case WIFI_REASON_BEACON_TIMEOUT:
                printf("Beacon timeout");
                break;
            case WIFI_REASON_NO_AP_FOUND:
                printf("No access point found");
                break;
            case WIFI_REASON_AUTH_FAIL:
                printf("Authentication failed");
                break;
            case WIFI_REASON_ASSOC_FAIL:
                printf("Association failed");
                break;
            case WIFI_REASON_HANDSHAKE_TIMEOUT:
                printf("Handshake timeout");
                break;
            case WIFI_REASON_CONNECTION_FAIL:
                printf("Connection failed");
                break;
            default:
                printf("Unknown reason");
                break;
        }
        printf("\n");
        
        if (s_retry_num < s_current_wifi_config.max_retry) {
            esp_wifi_connect();
            s_retry_num++;
            printf("Retry to connect to the access point (attempt %d/%d)\n", 
                   s_retry_num, s_current_wifi_config.max_retry);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        printf("Connection to the access point failed (attempt %d/%d)\n", 
               s_retry_num, s_current_wifi_config.max_retry);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

/**
 * @brief Initialize WiFi in station mode
 */
static esp_err_t wifi_init_sta_internal(void)
{
    if (s_wifi_initialized) {
        return ESP_OK;
    }

    s_wifi_event_group = xEventGroupCreate();
    if (s_wifi_event_group == NULL) {
        return ESP_ERR_NO_MEM;
    }

    esp_err_t ret = esp_netif_init();
    if (ret != ESP_OK) return ret;

    ret = esp_event_loop_create_default();
    if (ret != ESP_OK) return ret;

    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ret = esp_wifi_init(&cfg);
    if (ret != ESP_OK) return ret;

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ret = esp_event_handler_instance_register(WIFI_EVENT,
                                            ESP_EVENT_ANY_ID,
                                            &wifi_event_handler,
                                            NULL,
                                            &instance_any_id);
    if (ret != ESP_OK) return ret;

    ret = esp_event_handler_instance_register(IP_EVENT,
                                            IP_EVENT_STA_GOT_IP,
                                            &wifi_event_handler,
                                            NULL,
                                            &instance_got_ip);
    if (ret != ESP_OK) return ret;

    wifi_config_t wifi_config = {0};
    
    // Set up station configuration
    strncpy((char*)wifi_config.sta.ssid, s_current_wifi_config.ssid, sizeof(wifi_config.sta.ssid) - 1);
    strncpy((char*)wifi_config.sta.password, s_current_wifi_config.password, sizeof(wifi_config.sta.password) - 1);
    wifi_config.sta.threshold.authmode = WIFI_AUTH_WPA2_PSK;
    wifi_config.sta.pmf_cfg.capable = true;
    wifi_config.sta.pmf_cfg.required = false;

    ret = esp_wifi_set_mode(WIFI_MODE_STA);
    if (ret != ESP_OK) return ret;

    ret = esp_wifi_set_config(WIFI_IF_STA, &wifi_config);
    if (ret != ESP_OK) return ret;

    ret = esp_wifi_start();
    if (ret != ESP_OK) return ret;

    s_wifi_initialized = true;
    printf("WiFi initialized in station mode.\n");
    
    return ESP_OK;
}

esp_err_t esp32_mac_ip_init(void)
{
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
        ESP_ERROR_CHECK(nvs_flash_erase());
        ret = nvs_flash_init();
    }
    return ret;
}

esp_err_t esp32_mac_ip_get_mac(esp_mac_type_t mac_type, mac_info_t *mac_info)
{
    if (mac_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    esp_err_t ret = esp_read_mac(mac_info->mac, mac_type);
    if (ret != ESP_OK) {
        return ret;
    }

    // Format MAC address as string
    snprintf(mac_info->mac_string, sizeof(mac_info->mac_string),
             "%02X:%02X:%02X:%02X:%02X:%02X",
             mac_info->mac[0], mac_info->mac[1], mac_info->mac[2],
             mac_info->mac[3], mac_info->mac[4], mac_info->mac[5]);

    return ESP_OK;
}

esp_err_t esp32_mac_ip_connect_wifi(const esp32_mac_ip_wifi_config_t *wifi_config, ip_info_t *ip_info)
{
    if (wifi_config == NULL || ip_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Store WiFi configuration
    memcpy(&s_current_wifi_config, wifi_config, sizeof(esp32_mac_ip_wifi_config_t));
    s_retry_num = 0;

    // Initialize WiFi if not already done
    esp_err_t ret = wifi_init_sta_internal();
    if (ret != ESP_OK) {
        return ret;
    }

    // Wait a moment before attempting to connect to allow full initialization
    printf("Waiting for WiFi to initialize...\n");
    vTaskDelay(pdMS_TO_TICKS(2000)); // Wait 2 seconds

    // Wait for WiFi connection
    EventBits_t bits = xEventGroupWaitBits(s_wifi_event_group,
            WIFI_CONNECTED_BIT | WIFI_FAIL_BIT,
            pdFALSE,
            pdFALSE,
            portMAX_DELAY);

    if (bits & WIFI_CONNECTED_BIT) {
        printf("Connected to SSID: %s\n", wifi_config->ssid);
        
        // Get IP information
        esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
        if (netif != NULL) {
            esp_netif_ip_info_t esp_ip_info;
            ret = esp_netif_get_ip_info(netif, &esp_ip_info);
            if (ret == ESP_OK) {
                ip_info->ip = esp_ip_info.ip;
                ip_info->netmask = esp_ip_info.netmask;
                ip_info->gateway = esp_ip_info.gw;
                
                // Format IP addresses as strings
                snprintf(ip_info->ip_string, sizeof(ip_info->ip_string), IPSTR, IP2STR(&esp_ip_info.ip));
                snprintf(ip_info->netmask_string, sizeof(ip_info->netmask_string), IPSTR, IP2STR(&esp_ip_info.netmask));
                snprintf(ip_info->gateway_string, sizeof(ip_info->gateway_string), IPSTR, IP2STR(&esp_ip_info.gw));
                
                return ESP_OK;
            }
        }
        return ESP_FAIL;
    } else if (bits & WIFI_FAIL_BIT) {
        printf("Failed to connect to SSID: %s\n", wifi_config->ssid);
        printf("Error: Unable to connect to WiFi network after %d retries\n", wifi_config->max_retry);
        return ESP_FAIL;
    } else {
        printf("UNEXPECTED EVENT");
        return ESP_FAIL;
    }
}

esp_err_t esp32_mac_ip_get_network_info(const esp32_mac_ip_wifi_config_t *wifi_config, 
                                       esp_mac_type_t mac_type, 
                                       network_info_t *network_info)
{
    if (wifi_config == NULL || network_info == NULL) {
        return ESP_ERR_INVALID_ARG;
    }

    // Get MAC address
    esp_err_t ret = esp32_mac_ip_get_mac(mac_type, &network_info->mac);
    if (ret != ESP_OK) {
        return ret;
    }

    // Copy SSID
    strncpy(network_info->ssid, wifi_config->ssid, sizeof(network_info->ssid) - 1);
    network_info->ssid[sizeof(network_info->ssid) - 1] = '\0';

    // Connect to WiFi and get IP
    ret = esp32_mac_ip_connect_wifi(wifi_config, &network_info->ip);
    if (ret == ESP_OK) {
        network_info->wifi_connected = true;
    } else {
        network_info->wifi_connected = false;
        memset(&network_info->ip, 0, sizeof(ip_info_t));
    }

    return ESP_OK;
}

void esp32_mac_ip_print_mac(const mac_info_t *mac_info)
{
    if (mac_info == NULL) {
        return;
    }

    printf("\n=== Device MAC Address ===\n");
    printf("ESP32 MAC Address: %s\n", mac_info->mac_string);
    printf("==========================\n");
}

void esp32_mac_ip_print_ip(const ip_info_t *ip_info, const char *ssid)
{
    if (ip_info == NULL) {
        return;
    }

    printf("\n=== WiFi Information ===\n");
    printf("ESP32 IP Address: %s\n", ip_info->ip_string);
    if (ssid != NULL) {
        printf("WiFi SSID: %s\n", ssid);
    }
    printf("Netmask: %s\n", ip_info->netmask_string);
    printf("Gateway: %s\n", ip_info->gateway_string);
    printf("==============================\n");
}

void esp32_mac_ip_print_network_info(const network_info_t *network_info)
{
    if (network_info == NULL) {
        return;
    }

    esp32_mac_ip_print_mac(&network_info->mac);
    
    if (network_info->wifi_connected) {
        esp32_mac_ip_print_ip(&network_info->ip, network_info->ssid);
    } else {
        printf("\n=== WiFi Information ===\n");
        printf("WiFi Connection: Failed\n");
        printf("SSID: %s\n", network_info->ssid);
        printf("==============================\n");
    }
}

bool esp32_mac_ip_is_wifi_connected(void)
{
    if (!s_wifi_initialized) {
        return false;
    }

    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif == NULL) {
        return false;
    }

    esp_netif_ip_info_t ip_info;
    return (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK);
}

esp_err_t esp32_mac_ip_disconnect_wifi(void)
{
    if (!s_wifi_initialized) {
        return ESP_OK;
    }

    esp_err_t ret = esp_wifi_disconnect();
    if (ret != ESP_OK) {
        return ret;
    }

    ret = esp_wifi_stop();
    if (ret != ESP_OK) {
        return ret;
    }

    ret = esp_wifi_deinit();
    if (ret != ESP_OK) {
        return ret;
    }

    s_wifi_initialized = false;
    
    if (s_wifi_event_group != NULL) {
        vEventGroupDelete(s_wifi_event_group);
        s_wifi_event_group = NULL;
    }

    printf("WiFi disconnected and deinitialized.\n");
    return ESP_OK;
}

esp_err_t esp32_mac_ip_deinit(void)
{
    esp_err_t ret = esp32_mac_ip_disconnect_wifi();
    if (ret != ESP_OK) {
        return ret;
    }

    // Note: We don't deinitialize NVS flash as it might be used by other components
    return ESP_OK;
}
