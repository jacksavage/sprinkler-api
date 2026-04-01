# Sprinkler API

HTTP API for controlling 3 irrigation zones, running on a [Seeed XIAO ESP32-C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/).

## API

### `POST /api/schedule`

Runs zones sequentially in the order given. Send an empty array to stop all zones immediately.

**Request**
```json
[
  { "zone_id": 1, "duration_seconds": 300 },
  { "zone_id": 2, "duration_seconds": 600 }
]
```

**Response**
```json
{ "message": "Schedule started" }
```

Zone IDs are `1`, `2`, or `3`. Zones run one at a time; a new request cancels any running schedule before starting the new one.

### Zone → GPIO mapping

| Zone | XIAO pin | GPIO |
|------|----------|------|
| 1    | D0       | 2    |
| 2    | D1       | 3    |
| 3    | D2       | 4    |

Connect relay module inputs to D0–D2. Pin mapping can be changed in `include/config.h`.

---

## Setup with PlatformIO

### 1. Install PlatformIO

- **VS Code**: install the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
- **CLI**: `pip install platformio`

### 2. Configure WiFi credentials

Edit `include/config.h`:
```cpp
#define WIFI_SSID     "your_network"
#define WIFI_PASSWORD "your_password"
```

### 3. Build and upload

Connect the XIAO ESP32-C3 via USB-C, then:

**VS Code**: click the **Upload** arrow (→) in the PlatformIO toolbar.

**CLI**:
```bash
pio run --target upload
```

### 4. Find the IP address

Open the serial monitor (115200 baud) immediately after boot:

```bash
pio device monitor
```

You'll see:
```
Connected! IP: 192.168.1.42
Server started
```

Use that IP to reach the API, e.g. `http://192.168.1.42/api/schedule`.

---

## Frontend

The React/TypeScript frontend lives in `client/` and is hosted separately. Point its API base URL at the ESP32's IP address.
