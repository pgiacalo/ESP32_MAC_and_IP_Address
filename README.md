# ESP32 MAC and IP Address Library

A comprehensive ESP32 library for retrieving MAC addresses and IP addresses with easy WiFi connectivity management. This library provides a clean, reusable API that can be easily integrated into any ESP32 project.

## 🚀 Features

- **MAC Address Retrieval**: Get MAC addresses for different interfaces (WiFi STA, SoftAP, etc.)
- **WiFi Connectivity**: Simple WiFi connection management with retry logic
- **IP Address Management**: Automatic IP address retrieval and formatting
- **Comprehensive Output**: Formatted display of all network information
- **Error Handling**: Detailed error messages and proper error codes
- **Resource Management**: Proper initialization and cleanup
- **Easy Integration**: Clean API designed for reuse across projects

## 📚 Library Design

The library is structured into two main files:

- **`esp32_mac_ip.h`**: Header file with function declarations and data structures
- **`esp32_mac_ip.c`**: Implementation file with all functionality

### Key Data Structures

```c
// WiFi configuration
typedef struct {
    char ssid[32];
    char password[64];
    uint8_t max_retry;
} esp32_mac_ip_wifi_config_t;

// MAC address information
typedef struct {
    uint8_t mac[6];
    char mac_string[18]; // "XX:XX:XX:XX:XX:XX" format
} mac_info_t;

// IP address information
typedef struct {
    esp_ip4_addr_t ip;
    esp_ip4_addr_t netmask;
    esp_ip4_addr_t gateway;
    char ip_string[16]; // "XXX.XXX.XXX.XXX" format
    char netmask_string[16];
    char gateway_string[16];
} ip_info_t;

// Complete network information
typedef struct {
    mac_info_t mac;
    ip_info_t ip;
    bool wifi_connected;
    char ssid[32];
} network_info_t;
```

## 🔧 API Reference

### Core Functions

#### `esp32_mac_ip_init()`
Initialize the library. Must be called before any other functions.
```c
esp_err_t esp32_mac_ip_init(void);
```

#### `esp32_mac_ip_get_mac()`
Retrieve MAC address for specified interface type.
```c
esp_err_t esp32_mac_ip_get_mac(esp_mac_type_t mac_type, mac_info_t *mac_info);
```

#### `esp32_mac_ip_connect_wifi()`
Connect to WiFi network and retrieve IP address.
```c
esp_err_t esp32_mac_ip_connect_wifi(const esp32_mac_ip_wifi_config_t *wifi_config, ip_info_t *ip_info);
```

#### `esp32_mac_ip_get_network_info()`
Get complete network information (MAC + IP) in one call.
```c
esp_err_t esp32_mac_ip_get_network_info(const esp32_mac_ip_wifi_config_t *wifi_config, 
                                       esp_mac_type_t mac_type, 
                                       network_info_t *network_info);
```

### Utility Functions

#### `esp32_mac_ip_print_mac()`
Print MAC address information to console.
```c
void esp32_mac_ip_print_mac(const mac_info_t *mac_info);
```

#### `esp32_mac_ip_print_ip()`
Print IP address information to console.
```c
void esp32_mac_ip_print_ip(const ip_info_t *ip_info, const char *ssid);
```

#### `esp32_mac_ip_print_network_info()`
Print complete network information to console.
```c
void esp32_mac_ip_print_network_info(const network_info_t *network_info);
```

#### `esp32_mac_ip_is_wifi_connected()`
Check if WiFi is currently connected.
```c
bool esp32_mac_ip_is_wifi_connected(void);
```

#### `esp32_mac_ip_disconnect_wifi()`
Disconnect from WiFi network.
```c
esp_err_t esp32_mac_ip_disconnect_wifi(void);
```

#### `esp32_mac_ip_deinit()`
Clean up resources and deinitialize the library.
```c
esp_err_t esp32_mac_ip_deinit(void);
```

## 💡 Quick Start

### Basic Usage

```c
#include "esp32_mac_ip.h"

void app_main(void)
{
    // Initialize the library
    esp_err_t ret = esp32_mac_ip_init();
    if (ret != ESP_OK) {
        printf("Failed to initialize library: %s\n", esp_err_to_name(ret));
        return;
    }

    // Configure WiFi
    esp32_mac_ip_wifi_config_t wifi_config = {
        .ssid = "YourNetworkName",
        .password = "YourPassword",
        .max_retry = 5
    };

    // Get complete network information
    network_info_t network_info;
    ret = esp32_mac_ip_get_network_info(&wifi_config, ESP_MAC_WIFI_STA, &network_info);
    if (ret != ESP_OK) {
        printf("Failed to get network information: %s\n", esp_err_to_name(ret));
        return;
    }

    // Print all information
    esp32_mac_ip_print_network_info(&network_info);
}
```

### Advanced Usage

```c
// Get only MAC address
mac_info_t mac_info;
esp32_mac_ip_get_mac(ESP_MAC_WIFI_STA, &mac_info);
esp32_mac_ip_print_mac(&mac_info);

// Connect to WiFi and get IP separately
ip_info_t ip_info;
esp32_mac_ip_wifi_config_t wifi_config = {
    .ssid = "MyNetwork",
    .password = "MyPassword",
    .max_retry = 3
};
esp32_mac_ip_connect_wifi(&wifi_config, &ip_info);
esp32_mac_ip_print_ip(&ip_info, wifi_config.ssid);

// Check connection status
if (esp32_mac_ip_is_wifi_connected()) {
    printf("WiFi is connected!\n");
}
```

## ⚙️ Configuration

Before building and flashing, configure your WiFi credentials in `main.c`:

```c
#define WIFI_SSID      "YourNetworkName"
#define WIFI_PASS      "YourPassword"
#define WIFI_MAXIMUM_RETRY  5
```

## 🔨 Building and Flashing

1. Make sure you have ESP-IDF installed and configured
2. Build the project:
   ```bash
   idf.py build
   ```
3. Flash to your ESP32:
   ```bash
   idf.py flash monitor
   ```

## 📤 Expected Output

When the program runs successfully, you should see output similar to:

```
WiFi initialized in station mode.
Waiting for WiFi to initialize...
Connected to SSID: YourNetworkName

=== Device MAC Address ===
ESP32 MAC Address: FC:01:2C:E3:CA:94
==========================

=== WiFi Information ===
ESP32 IP Address: 192.168.86.49
WiFi SSID: YourNetworkName
Netmask: 255.255.255.0
Gateway: 192.168.86.1
==============================
```

## 🔗 Integration Guide

### Adding to Your Project

1. **Copy Library Files**: Copy `esp32_mac_ip.h` and `esp32_mac_ip.c` to your project's `main` directory
2. **Update CMakeLists.txt**: Add the library file to your component:
   ```cmake
   idf_component_register(SRCS "esp32_mac_ip.c" "main.c"
                         INCLUDE_DIRS "."
                         REQUIRES esp_wifi esp_netif nvs_flash)
   ```
3. **Include Header**: Add `#include "esp32_mac_ip.h"` to your source files
4. **Initialize**: Call `esp32_mac_ip_init()` before using any library functions

### Dependencies

The library requires these ESP-IDF components:
- `esp_wifi`: WiFi functionality
- `esp_netif`: Network interface management  
- `nvs_flash`: Non-volatile storage for WiFi credentials
- `esp_event`: Event handling system
- `freertos`: Real-time operating system components

## 🛠️ Error Handling

The library includes comprehensive error handling:

- **WiFi Connection Failures**: Detailed disconnect reason messages with retry mechanism
- **Network Interface Issues**: Proper error codes and initialization checks
- **IP Address Retrieval**: Graceful handling of network configuration failures
- **Resource Management**: Proper cleanup on errors

### Common Error Scenarios

```c
// Check for connection failures
if (!esp32_mac_ip_is_wifi_connected()) {
    printf("WiFi connection failed. Check credentials and network availability.\n");
}

// Handle library initialization errors
esp_err_t ret = esp32_mac_ip_init();
if (ret != ESP_OK) {
    printf("Library initialization failed: %s\n", esp_err_to_name(ret));
    return;
}
```

## 🎯 Use Cases

This library is perfect for:
- **Device Identification**: Getting unique MAC addresses for device registration
- **Network Configuration**: Automatically connecting to WiFi and getting IP addresses
- **IoT Applications**: Quick network setup for ESP32-based IoT devices
- **Debugging**: Network troubleshooting and status monitoring
- **Device Management**: Remote device identification and network status

## 📝 License

This project is open source and available under the MIT License.
