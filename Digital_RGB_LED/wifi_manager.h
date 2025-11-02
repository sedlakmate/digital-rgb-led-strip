// wifi_manager.h
//
// WiFi connection management for ESP32 using WiFiManager library.
// Provides auto-connect functionality with fallback to configuration portal.
//
// Usage:
//   - Call wifiSetup() in setup() to initialize WiFi connection
//   - Device will attempt to connect using stored credentials
//   - If connection fails, creates AP for configuration
//   - Configuration portal accessible at default IP (192.168.4.1)
//
// Configuration:
//   - Set AP name and timeout in config.h or config_override.h
//

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#ifdef ESP32

#include <WiFi.h>

// Initialize WiFi connection
// Creates configuration portal if connection fails
void wifiSetup();

// Get current WiFi connection status
bool wifiIsConnected();

// Get local IP address
String wifiGetIP();

#endif  // ESP32

#endif  // WIFI_MANAGER_H
