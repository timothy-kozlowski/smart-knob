#pragma once

// Template for secrets.h, which is gitignored.
// Setup: copy this file to secrets.h in the same folder and fill in real values.
// Never put real credentials in this file; it is committed to git.

// Wi-Fi (2.4 GHz only: the ESP32-S3 radio has no 5 GHz support)
#define SECRET_WIFI_SSID        "your-wifi-ssid"
#define SECRET_WIFI_PASSWORD    "your-wifi-password"

// Web API keys: add one #define per service, prefixed with SECRET_
#define SECRET_API_KEY_EXAMPLE  "your-api-key"
