# Sprinkler API

HTTP API for controlling 3 irrigation zones, running on a [Seeed XIAO ESP32-C3](https://wiki.seeedstudio.com/XIAO_ESP32C3_Getting_Started/).

## API

### `POST /api/schedule`

Runs zones sequentially in the order given. Send an empty array to stop all zones immediately.

**Request**
```json
[
  { "zone_id": 0, "duration_seconds": 300 },
  { "zone_id": 1, "duration_seconds": 600 }
]
```

**Response**
```json
{ "message": "Schedule started" }
```

Zone IDs are `0`, `1`, or `2`. Zones run one at a time; a new request cancels any running schedule before starting the new one.

### Zone → GPIO mapping

| Zone | XIAO pin | GPIO |
|------|----------|------|
| 0    | D0       | 2    |
| 1    | D1       | 3    |
| 2    | D2       | 4    |

Connect relay module inputs to D0–D2. Pin mapping can be changed in `include/config.h`.

---

## Setup with PlatformIO

### 1. Install PlatformIO

- **VS Code**: install the [PlatformIO IDE extension](https://platformio.org/install/ide?install=vscode)
- **CLI**: `pip install platformio`

### 2. Set WiFi credentials via 1Password CLI

WiFi credentials are injected at build time via environment variables (`${sysenv.WIFI_SSID}` / `${sysenv.WIFI_PASSWORD}` in `platformio.ini`).

> **VS Code GUI caveat:** the PlatformIO extension only sees env vars that existed when VS Code was launched. The fix is to set the vars in PowerShell *then* open VS Code from that same session — it will inherit them, and the Upload button works normally.

**PowerShell — set creds and open VS Code:**
```powershell
$env:WIFI_SSID     = op read "op://Private/sasquatch wifi/network name"
$env:WIFI_PASSWORD = op read "op://Private/sasquatch wifi/wireless network password"
code .
```

Then click the **Upload** arrow (→) in the PlatformIO toolbar as usual.

**PowerShell — CLI upload (no VS Code needed):**
```powershell
$env:WIFI_SSID     = op read "op://Private/sasquatch wifi/network name"
$env:WIFI_PASSWORD = op read "op://Private/sasquatch wifi/wireless network password"
$env:PATH         += ";${env:USERPROFILE}\.platformio\penv\Scripts"
pio run --target upload
```

### 3. Find the IP address

Open the serial monitor (115200 baud) immediately after boot:

```powershell
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
