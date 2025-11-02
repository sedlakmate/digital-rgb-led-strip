// web_server.cpp
//
// Implementation of asynchronous web server for LED strip control.


// Include ESPAsyncWebServer BEFORE any WiFiManager includes to avoid HTTP method conflicts
#include <ESPAsyncWebServer.h>
#include "web_server.h"
#include "debug.h"
#include "config.h"
#include "wifi_manager.h"

// External variables from main sketch
extern int brightness;
extern float bpm;
extern float waveLengthScale;
extern uint8_t gPaletteIndex;
extern const uint8_t gPaletteCount;
extern void setLeds();
extern int calculateDelayMillis();
extern int delayMillis;

AsyncWebServer server(WEB_SERVER_PORT);

void echoRequestToSerial(AsyncWebServerRequest *request) {
  dbg::println("\n========== Incoming HTTP Request ==========");

  // Request method and URL
  dbg::print("[Request] Method: ");
  switch (request->method()) {
    case HTTP_GET:    dbg::println("GET"); break;
    case HTTP_POST:   dbg::println("POST"); break;
    case HTTP_DELETE: dbg::println("DELETE"); break;
    case HTTP_PUT:    dbg::println("PUT"); break;
    case HTTP_PATCH:  dbg::println("PATCH"); break;
    case HTTP_HEAD:   dbg::println("HEAD"); break;
    case HTTP_OPTIONS: dbg::println("OPTIONS"); break;
    default:          dbg::println("UNKNOWN"); break;
  }

  dbg::print("[Request] URL: ");
  dbg::println(request->url());

  dbg::print("[Request] Host: ");
  dbg::println(request->host());

  dbg::print("[Request] Content Type: ");
  dbg::println(request->contentType());

  // Query parameters
  int params = request->params();
  if (params > 0) {
    dbg::println("[Request] Parameters:");
    for (int i = 0; i < params; i++) {
      const AsyncWebParameter* p = request->getParam(i);
      dbg::print("  - ");
      dbg::print(p->name());
      dbg::print(" = ");
      dbg::println(p->value());
    }
  }

  // Headers
  int headers = request->headers();
  if (headers > 0) {
    dbg::println("[Request] Headers:");
    for (int i = 0; i < headers; i++) {
      const AsyncWebHeader* h = request->getHeader(i);
      dbg::print("  - ");
      dbg::print(h->name());
      dbg::print(": ");
      dbg::println(h->value());
    }
  }

  dbg::println("==========================================\n");
}

void webServerSetup() {
  dbg::println("[WebServer] Setting up web server...");

  // Root endpoint - HTML status page
  server.on("/", HTTP_GET, [](AsyncWebServerRequest *request) {
    echoRequestToSerial(request);

    String html = "<!DOCTYPE html><html><head>";
    html += "<title>LED Strip Control</title>";
    html += "<meta name='viewport' content='width=device-width, initial-scale=1'>";
    html += "<style>body{font-family:Arial;margin:20px;background:#1a1a1a;color:#fff;}";
    html += "h1{color:#4CAF50;}";
    html += ".card{background:#2a2a2a;padding:20px;margin:10px 0;border-radius:8px;}";
    html += ".value{color:#4CAF50;font-weight:bold;}";
    html += "a{color:#2196F3;text-decoration:none;}";
    html += "a:hover{text-decoration:underline;}</style></head><body>";
    html += "<h1>LED Strip Control Panel</h1>";
    html += "<div class='card'><h2>Current Status</h2>";
    html += "<p>IP Address: <span class='value'>" + wifiGetIP() + "</span></p>";
    html += "<p>Brightness: <span class='value'>" + String(brightness) + "</span></p>";
    html += "<p>BPM: <span class='value'>" + String(bpm) + "</span></p>";
    html += "<p>Wave Length Scale: <span class='value'>" + String(waveLengthScale) + "</span></p>";
    html += "<p>Palette Index: <span class='value'>" + String(gPaletteIndex) + "</span></p>";
    html += "<p>Delay (ms): <span class='value'>" + String(delayMillis) + "</span></p>";
    html += "</div>";
    html += "<div class='card'><h2>API Endpoints</h2>";
    html += "<p><a href='/api/status'>GET /api/status</a> - Get JSON status</p>";
    html += "<p><strong>POST /api/led</strong> - Control LED parameters (combine multiple in one request):</p>";
    html += "<ul>";
    html += "<li><code>brightness</code> - Set brightness (0-" + String(BRIGHTNESS_MAX) + ")</li>";
    html += "<li><code>bpm</code> - Set BPM (" + String(BPM_MIN) + "-" + String(BPM_MAX) + ")</li>";
    html += "<li><code>palette</code> - Set palette (0-" + String(gPaletteCount - 1) + ")</li>";
    html += "<li><code>wavelength</code> - Set wave length (" + String(WAVE_LENGTH_SCALE_MIN) + "-" + String(WAVE_LENGTH_SCALE_MAX) + ")</li>";
    html += "</ul>";
    html += "<p>Examples: <code>/api/led?brightness=200</code> or <code>/api/led?brightness=200&bpm=10&palette=3</code></p>";
    html += "</div></body></html>";

    request->send(200, "text/html", html);
  });

  // Status endpoint - JSON
  server.on("/api/status", HTTP_GET, [](AsyncWebServerRequest *request) {
    echoRequestToSerial(request);

    String json = "{";
    json += "\"ip\":\"" + wifiGetIP() + "\",";
    json += "\"brightness\":" + String(brightness) + ",";
    json += "\"bpm\":" + String(bpm) + ",";
    json += "\"waveLengthScale\":" + String(waveLengthScale) + ",";
    json += "\"paletteIndex\":" + String(gPaletteIndex) + ",";
    json += "\"paletteCount\":" + String(gPaletteCount) + ",";
    json += "\"delayMillis\":" + String(delayMillis);
    json += "}";

    request->send(200, "application/json", json);
  });

  // Unified LED control endpoint
  server.on("/api/led", HTTP_POST, [](AsyncWebServerRequest *request) {
    echoRequestToSerial(request);

    bool updated = false;
    String response = "{\"status\":\"ok\"";

    // Brightness control
    if (request->hasParam("brightness")) {
      int newBrightness = request->getParam("brightness")->value().toInt();
      newBrightness = constrain(newBrightness, BRIGHTNESS_MIN, BRIGHTNESS_MAX);
      brightness = newBrightness;
      FastLED.setBrightness(brightness);
      FastLED.show();

      dbg::print("[WebServer] Brightness set to: ");
      dbg::println(brightness);

      response += ",\"brightness\":" + String(brightness);
      updated = true;
    }

    // BPM control
    if (request->hasParam("bpm")) {
      float newBPM = request->getParam("bpm")->value().toFloat();
      newBPM = constrain(newBPM, BPM_MIN, BPM_MAX);
      bpm = newBPM;
      delayMillis = calculateDelayMillis();

      dbg::print("[WebServer] BPM set to: ");
      dbg::println(bpm);

      response += ",\"bpm\":" + String(bpm);
      updated = true;
    }

    // Palette control
    if (request->hasParam("palette")) {
      int newIndex = request->getParam("palette")->value().toInt();
      if (newIndex >= 0 && newIndex < gPaletteCount) {
        gPaletteIndex = newIndex;
        extern CRGBPalette256 currentPalette;
        extern const TProgmemRGBPalette16* const* gPalettes;
        currentPalette = CRGBPalette16(*gPalettes[gPaletteIndex]);
        setLeds();

        dbg::print("[WebServer] Palette index set to: ");
        dbg::println(gPaletteIndex);

        response += ",\"paletteIndex\":" + String(gPaletteIndex);
        updated = true;
      } else {
        request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"Invalid palette index (0-" + String(gPaletteCount - 1) + ")\"}");
        return;
      }
    }

    // Wave length scale control
    if (request->hasParam("wavelength")) {
      float newScale = request->getParam("wavelength")->value().toFloat();
      newScale = constrain(newScale, WAVE_LENGTH_SCALE_MIN, WAVE_LENGTH_SCALE_MAX);
      waveLengthScale = newScale;
      setLeds();

      dbg::print("[WebServer] Wave length scale set to: ");
      dbg::println(waveLengthScale);

      response += ",\"waveLengthScale\":" + String(waveLengthScale);
      updated = true;
    }

    if (updated) {
      response += "}";
      request->send(200, "application/json", response);
    } else {
      request->send(400, "application/json", "{\"status\":\"error\",\"message\":\"No valid parameters provided. Use: brightness, bpm, palette, or wavelength\"}");
    }
  });

  // 404 handler
  server.onNotFound([](AsyncWebServerRequest *request) {
    echoRequestToSerial(request);
    request->send(404, "application/json", "{\"status\":\"error\",\"message\":\"Not found\"}");
  });

  // Start server
  server.begin();
  dbg::println("[WebServer] Web server started on port " + String(WEB_SERVER_PORT));
  dbg::println("[WebServer] Access at: http://" + wifiGetIP());
}
