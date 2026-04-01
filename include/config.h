#pragma once

// WiFi credentials
#define WIFI_SSID     "your_ssid_here"
#define WIFI_PASSWORD "your_password_here"

// Zone to GPIO pin mapping (XIAO ESP32-C3)
// Zone 1 -> D0 (GPIO2)
// Zone 2 -> D1 (GPIO3)
// Zone 3 -> D2 (GPIO4)
const int NUM_ZONES = 3;
const int ZONE_PINS[NUM_ZONES + 1] = {-1, 2, 3, 4}; // 1-indexed; index 0 unused
