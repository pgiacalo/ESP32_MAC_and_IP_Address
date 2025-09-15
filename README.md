# ESP32 MAC Address and IP Address Example

This ESP32 project demonstrates how to retrieve both the MAC address and IP address of an ESP32 device.

## Features

- Retrieves and displays the WiFi station MAC address
- Connects to a WiFi network
- Retrieves and displays the assigned IP address, netmask, and gateway
- Comprehensive network information output

## Configuration

No pre-configuration is required! The program will prompt you for WiFi credentials when it runs.

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

## User Input

The program uses UART (serial communication) to receive user input. When prompted:
- Type your WiFi SSID and press Enter
- Type your WiFi password and press Enter
- You can use backspace to correct typing errors
- The input is displayed in real-time as you type

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
