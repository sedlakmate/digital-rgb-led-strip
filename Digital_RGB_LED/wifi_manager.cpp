// wifi_manager.cpp
//
// Implementation of WiFi connection management for ESP32.

#ifdef ESP32

#include "debug.h"
#include "config.h"
#include <WiFi.h>
#include <WiFiManager.h>

WiFiManager wifiManager;

void wifiSetup() {
  dbg::println("[WiFi] Initializing WiFi Manager...");

  // Set timeout for configuration portal (in seconds)
  wifiManager.setConfigPortalTimeout(WIFI_CONFIG_PORTAL_TIMEOUT);

  // Automatically connect using saved credentials
  // If connection fails, starts access point with specified name
  dbg::print("[WiFi] Attempting to connect or start config portal (SSID: ");
  dbg::print(WIFI_AP_NAME);
  dbg::println(")");

  if (!wifiManager.autoConnect(WIFI_AP_NAME, WIFI_AP_PASSWORD)) {
    dbg::println("[WiFi] Failed to connect and config portal timeout reached");
    dbg::println("[WiFi] Restarting device...");
    delay(3000);
    ESP.restart();
  }

  // If we get here, WiFi is connected
  dbg::println("[WiFi] Successfully connected!");
  dbg::print("[WiFi] IP address: ");
  dbg::println(WiFi.localIP());
  dbg::print("[WiFi] SSID: ");
  dbg::println(WiFi.SSID());
}

bool wifiIsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String wifiGetIP() {
  return WiFi.localIP().toString();
}

#endif  // ESP32
