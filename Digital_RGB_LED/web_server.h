// web_server.h
//
// Asynchronous web server for ESP32 LED strip control.
// Provides REST API endpoints and echoes all requests to Serial.
//
// Usage:
//   - Call webServerSetup() after WiFi is connected
//   - Server runs asynchronously in the background
//   - All requests are logged to Serial with full details
//
// API Endpoints:
//   GET  /              - HTML status page
//   GET  /api/status    - JSON status (brightness, BPM, palette, etc.)
//   POST /api/led       - Control LED parameters (supports multiple params in one request)
//                         Parameters: brightness, bpm, palette, wavelength
//                         Examples:
//                           /api/led?brightness=200
//                           /api/led?brightness=200&bpm=10
//                           /api/led?palette=3&wavelength=2.5
//

#ifndef WEB_SERVER_H
#define WEB_SERVER_H

#ifdef ESP32

#include <ESPAsyncWebServer.h>

// Initialize and start the web server
void webServerSetup();

// Helper function to echo request details to Serial
void echoRequestToSerial(AsyncWebServerRequest *request);

#endif  // ESP32

#endif  // WEB_SERVER_H
