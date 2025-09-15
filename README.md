# ESP32 MAC Address and IP Address Utility

This ESP32 project demonstrates how to retrieve both the MAC address and IP address of an ESP32 device.

## Features

- Retrieves and displays the WiFi station MAC address
- Connects to a WiFi network
- Retrieves and displays the assigned IP address, netmask, and gateway
- Comprehensive network information output

## Configuration

Before building and flashing, you must configure the WiFi credentials in `main.c`:

1. Open `main/main.c`
2. Find the WiFi configuration section
3. Update the following variables with your network details:
   - `WIFI_SSID`: Your WiFi network name
   - `WIFI_PASSWORD`: Your WiFi password

Example:
```c
#define WIFI_SSID "YourNetworkName"
#define WIFI_PASSWORD "YourPassword"
```

## Building and Flashing

1. Make sure you have ESP-IDF installed and configured
2. Build the project:
   ```bash
   idf.py build
   ```
3. Flash to your ESP32:
   ```bash
   idf.py flash monitor
   ```

## Expected Output

When the program runs successfully, you should see output similar to:

```
Connected to SSID: Aardvark

=== Device MAC Address ===
MAC Address: FC:01:2C:E3:CA:94
==========================

=== WiFi Information ===
ESP32 IP Address: 192.168.86.49
WiFi SSID: Aardvark
Netmask: 255.255.255.0
Gateway: 192.168.86.1
==============================
```

## Usage

Once configured and flashed, the program will:
1. Automatically connect to the configured WiFi network
2. Display the device MAC address
3. Show the assigned IP address and network information
4. Run continuously, displaying network status

## Components Used

- `esp_wifi`: WiFi functionality
- `esp_netif`: Network interface management
- `nvs_flash`: Non-volatile storage for WiFi credentials
- `esp_event`: Event handling system
- `freertos`: Real-time operating system components

## Error Handling

The program includes comprehensive error handling for:
- WiFi connection failures (with retry mechanism)
- Network interface initialization
- IP address retrieval failures

If WiFi connection fails after 5 retries, the program will display an error message but continue to show the MAC address.
