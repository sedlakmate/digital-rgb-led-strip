// wifi_manager.cpp
//
// Implementation of WiFi connection management for ESP32.

#include "debug.h"
#include "config.h"
#include <WiFi.h>
#include <WiFiManager.h>
#include <ESPmDNS.h>

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

  // Start mDNS responder
  dbg::print("[mDNS] Starting mDNS responder with hostname: ");
  dbg::print(MDNS_HOSTNAME);
  dbg::println(".local");

  // Give WiFi a moment to stabilize
  delay(100);

  if (MDNS.begin(MDNS_HOSTNAME)) {
    dbg::println("[mDNS] mDNS responder started successfully");
    dbg::print("[mDNS] Device accessible at: http://");
    dbg::print(MDNS_HOSTNAME);
    dbg::println(".local");
    dbg::print("[mDNS] Also accessible at: http://");
    dbg::println(WiFi.localIP());

    // Add service to mDNS-SD
    MDNS.addService("http", "tcp", WEB_SERVER_PORT);
    MDNS.addServiceTxt("http", "tcp", "name", MDNS_SERVICE_NAME);
    MDNS.addServiceTxt("http", "tcp", "version", "1.0");
    MDNS.addServiceTxt("http", "tcp", "path", "/");

    dbg::println("[mDNS] HTTP service advertised for discovery");
    dbg::println("[mDNS] Service name: " + String(MDNS_SERVICE_NAME));

    // Ensure mDNS announces itself
    delay(100);
  } else {
    dbg::println("[mDNS] ERROR: Failed to start mDNS responder!");
    dbg::println("[mDNS] Please access device via IP address: http://" + WiFi.localIP().toString());
  }
}

void wifiLoop() {
  // ESP32 mDNS handles updates automatically in the background
  // No explicit update call needed (unlike ESP8266)
}

bool wifiIsConnected() {
  return WiFi.status() == WL_CONNECTED;
}

String wifiGetIP() {
  return WiFi.localIP().toString();
}
