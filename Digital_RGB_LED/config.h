// Main configuration file for the project.
// To override anything locally, create a `config_override.h` in the same folder
// and define only the parameters you want to change. Do NOT track that file
// in version control.
//
// ──────────────────────────────────────────────────────────────
// HOW TO USE THIS FILE:
//   - Edit this file to set global defaults for your project.
//   - For local or per-device overrides, copy config_override_template.h to config_override.h
//     and define only the settings you want to change. This file is automatically included
//     at the end of this file if present.
//
// ──────────────────────────────────────────────────────────────
// PIN AND FEATURE CONFIGURATION:
//   - If a hardware pin macro (e.g., US_TRIG_PIN, BRIGHTNESS_KNOB_PIN) is defined,
//     the corresponding feature is enabled and its code is included in the build.
//   - If a pin is not defined, the feature is stubbed out and does nothing.
//
//   Example:
//     #define US_TRIG_PIN 7
//     #define US_ECHO_PIN 8
//     #define BRIGHTNESS_KNOB_PIN A0
//
// ──────────────────────────────────────────────────────────────
// KNOB AND SENSOR RANGES:
//   - KNOB_5V and KNOB_3_3V define the expected analogRead() max value for 5V and 3.3V systems.
//   - US_MAX_DISTANCE_CM and US_MIN_DISTANCE_CM set the usable range for the ultrasound sensor.
//
#ifndef CONFIG_H
#define CONFIG_H

#include <FastLED.h>
#include "palette.h"  // brings in PREDEFINED_PALETTES + typedefs

/* ─────────── Debug flag ─────────────── */
#define DEBUG 0  // set to 1 in config_override.h for verbose logs

/* ──────────── Hardware pins ──────────── */
/* If pins are defined, the corresponding device gets loaded */
#define LED_PIN 31  // Arduino digital pin
// #define US_TRIG_PIN 7  // Ultrasound TRIG pin
// #define US_ECHO_PIN 8  // Ultrasound ECHO pin
// #define BRIGHTNESS_KNOB_PIN A0  // Potentiometer for brightness
// #define BPM_KNOB_PIN A0         // Potentiometer for BPM
// #define WAVE_LENGTH_SCALE_KNOB_PIN A0 // Potentiometer for wave length scale

/* ────────── LED configuration ────────── */
#define LED_TYPE WS2812B
#define LED_STRIP_COLOR_CORRECTION TypicalLEDStrip
#define COLOR_ORDER GRB
#define NUM_LEDS 300  // total LEDs on the strip

/* ─────────── Animation defaults ─────── */
#define BRIGHTNESS 220  // 0-255, initial brightness
#define BPM 5.0         // initial beats per minute
#define WAVE_LENGTH_SCALE 1.0  // initial wave length scale
#define RESOLUTION 1    // animation resolution (higher = smoother, slower)
#define REVERSED false  // reverse LED order
#define PALETTE_INDEX 6 // default palette index (0-based)

/* ────────── Animation limits ────────── */
#define BRIGHTNESS_MIN 0
#define BRIGHTNESS_MAX 255
#define BRIGHTNESS_CHANGE_THRESHOLD 5  // minimum change to update brightness
#define BPM_MIN 0.01
#define BPM_MAX 35.0
#define BPM_CHANGE_THRESHOLD 0.6       // minimum change to update BPM
#define WAVE_LENGTH_SCALE_MIN 0.01
#define WAVE_LENGTH_SCALE_MAX 8.0
#define WAVE_LENGTH_SCALE_CHANGE_THRESHOLD 0.075  // minimum change to update scale

/* ────────── Sensor config ────────── */
// US_MAX_DISTANCE_CM significantly influences performance and blocking time.
// Lower values reduce blocking time. Examples:
// US_MAX_DISTANCE_CM = 50 cm => MAX_CM = 55 cm, blocking time ~3.2 ms if no echo.
// US_MAX_DISTANCE_CM = 40 cm => MAX_CM = 45 cm, blocking time ~2.6 ms if no echo.
// US_MAX_DISTANCE_CM = 30 cm => MAX_CM = 35 cm, blocking time ~2.0 ms if no echo.
#define US_MAX_DISTANCE_CM 40  // maximum distance to measure (cm)
#define US_MIN_DISTANCE_CM 5   // minimum distance to consider valid (cm)
#define KNOB_5V 1023           // analogRead() max value for 5V reference
#define KNOB_3_3V 675          // analogRead() max value for 3.3V reference

/* ────────── WiFi & Web Server (ESP32 only) ────────── */
#define WIFI_ENABLED false                        // Enable WiFi and web server features
#define WIFI_AP_NAME "LED-Strip-Setup"           // Access Point name for WiFi config portal
#define WIFI_AP_PASSWORD "ledstrip123"           // Access Point password (min 8 chars)
#define WIFI_CONFIG_PORTAL_TIMEOUT 180           // Config portal timeout in seconds (0 = no timeout)
#define WEB_SERVER_PORT 80                       // Web server port

/* ─────────── Local overrides ─────────── */
#ifdef __has_include
#if __has_include("config_override.h")
#include "config_override.h"
#endif
#endif

/* ─────────── Globals (defined in .ino) ─ */
extern CRGB leds[NUM_LEDS];
extern CRGBPalette256 currentPalette;
extern TBlendType currentBlending;

/* palette registry (table lives in palette.cpp, pointer lives in .ino) */
extern const TProgmemRGBPalette16* const* gPalettes;  // pointer to table
extern const uint8_t gPaletteCount;
extern uint8_t gPaletteIndex;

#endif  // CONFIG_H
