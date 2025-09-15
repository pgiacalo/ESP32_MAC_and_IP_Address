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

// WiFi configuration
#define WIFI_SSID      "Aardvark"
#define WIFI_PASS      "*******"
#define WIFI_MAXIMUM_RETRY  5


// Event group for WiFi connection status
static EventGroupHandle_t s_wifi_event_group;
#define WIFI_CONNECTED_BIT BIT0
#define WIFI_FAIL_BIT      BIT1

static int s_retry_num = 0;


static void event_handler(void* arg, esp_event_base_t event_base,
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
        
        if (s_retry_num < WIFI_MAXIMUM_RETRY) {
            esp_wifi_connect();
            s_retry_num++;
            printf("Retry to connect to the access point (attempt %d/%d)\n", s_retry_num, WIFI_MAXIMUM_RETRY);
        } else {
            xEventGroupSetBits(s_wifi_event_group, WIFI_FAIL_BIT);
        }
        printf("Connection to the access point failed (attempt %d/%d)\n", s_retry_num, WIFI_MAXIMUM_RETRY);
    } else if (event_base == IP_EVENT && event_id == IP_EVENT_STA_GOT_IP) {
        // printf("got ip:" IPSTR "\n", IP2STR(&((ip_event_got_ip_t*)event_data)->ip_info.ip));
        s_retry_num = 0;
        xEventGroupSetBits(s_wifi_event_group, WIFI_CONNECTED_BIT);
    }
}

void wifi_init_sta(void)
{
    s_wifi_event_group = xEventGroupCreate();

    ESP_ERROR_CHECK(esp_netif_init());
    ESP_ERROR_CHECK(esp_event_loop_create_default());
    esp_netif_create_default_wifi_sta();

    wifi_init_config_t cfg = WIFI_INIT_CONFIG_DEFAULT();
    ESP_ERROR_CHECK(esp_wifi_init(&cfg));

    esp_event_handler_instance_t instance_any_id;
    esp_event_handler_instance_t instance_got_ip;
    ESP_ERROR_CHECK(esp_event_handler_instance_register(WIFI_EVENT,
                                                        ESP_EVENT_ANY_ID,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_any_id));
    ESP_ERROR_CHECK(esp_event_handler_instance_register(IP_EVENT,
                                                        IP_EVENT_STA_GOT_IP,
                                                        &event_handler,
                                                        NULL,
                                                        &instance_got_ip));

    wifi_config_t wifi_config = {
        .sta = {
            .ssid = WIFI_SSID,
            .password = WIFI_PASS,
            .threshold.authmode = WIFI_AUTH_WPA2_PSK,
            .pmf_cfg = {
                .capable = true,
                .required = false
            },
        },
    };
    ESP_ERROR_CHECK(esp_wifi_set_mode(WIFI_MODE_STA) );
    ESP_ERROR_CHECK(esp_wifi_set_config(WIFI_IF_STA, &wifi_config) );
    ESP_ERROR_CHECK(esp_wifi_start() );

    printf("wifi_init_sta finished.\n");
    
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
        printf("Connected to SSID: %s\n", WIFI_SSID);
    } else if (bits & WIFI_FAIL_BIT) {
        printf("Failed to connect to SSID:%s\n", WIFI_SSID);
        printf("\n=== ESP32 WiFi Information ===\n");
        printf("WiFi Connection: Failed\n");
        printf("Error: Unable to connect to WiFi network after %d retries\n", WIFI_MAXIMUM_RETRY);
        printf("SSID: %s\n", WIFI_SSID);
        printf("==============================\n");
    } else {
        printf("UNEXPECTED EVENT");
    }
}

void app_main(void)
{
    // Initialize NVS
    esp_err_t ret = nvs_flash_init();
    if (ret == ESP_ERR_NVS_NO_FREE_PAGES || ret == ESP_ERR_NVS_NEW_VERSION_FOUND) {
      ESP_ERROR_CHECK(nvs_flash_erase());
      ret = nvs_flash_init();
    }
    ESP_ERROR_CHECK(ret);

    uint8_t mac[6];
    
    // Get MAC address for WiFi station interface
    esp_read_mac(mac, ESP_MAC_WIFI_STA);
    
    // Initialize WiFi and get IP address
    wifi_init_sta();
    
    // Report all information together at the end
    printf("\n=== Device MAC Address ===\n");
    printf("ESP32 MAC Address: %02X:%02X:%02X:%02X:%02X:%02X\n",
           mac[0], mac[1], mac[2], mac[3], mac[4], mac[5]);
    printf("==========================\n");
    
    // Get IP address information and display WiFi info (only if connection was successful)
    esp_netif_t *netif = esp_netif_get_handle_from_ifkey("WIFI_STA_DEF");
    if (netif != NULL) {
        esp_netif_ip_info_t ip_info;
        if (esp_netif_get_ip_info(netif, &ip_info) == ESP_OK) {
            printf("\n=== WiFi Information ===\n");
            printf("ESP32 IP Address: " IPSTR "\n", IP2STR(&ip_info.ip));
            printf("WiFi SSID: %s\n", WIFI_SSID);
            printf("Netmask: " IPSTR "\n", IP2STR(&ip_info.netmask));
            printf("Gateway: " IPSTR "\n", IP2STR(&ip_info.gw));
            printf("==============================\n");
        }
    }
}
