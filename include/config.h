#pragma once

// WiFi credentials are injected via build_flags in platformio.ini:
//   -DWIFI_SSID="${sysenv.WIFI_SSID}"
//   -DWIFI_PASSWORD="${sysenv.WIFI_PASSWORD}"

// Zone to GPIO pin mapping (XIAO ESP32-C3)
// Zone 0 -> D0 (GPIO2)
// Zone 1 -> D1 (GPIO3)
// Zone 2 -> D2 (GPIO4)
const int ZONE_PINS[] = {2, 3, 4};
constexpr int NUM_ZONES = sizeof(ZONE_PINS) / sizeof(ZONE_PINS[0]);
