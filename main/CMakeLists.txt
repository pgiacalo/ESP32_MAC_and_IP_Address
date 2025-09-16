idf_component_register(SRCS "esp32_mac_ip.c" "main.c"
                    INCLUDE_DIRS "."
                    REQUIRES esp_wifi esp_netif nvs_flash)
