// Digital_RGB_LED.ino
//
// Main entry point for the Digital_RGB_LED project.
//
// This sketch controls a WS2812B (NeoPixel) LED strip with support for:
//   - Multiple animation palettes
//   - Real-time control via potentiometer knobs (brightness, BPM, wave length scale)
//   - Optional ultrasound distance sensor for interactive effects
//   - Debug logging (enable via config)
//
// Configuration:
//   - All hardware and animation parameters are set in config.h or config_override.h
//   - Only define the pins/features you use; others are stubbed out
//
// Wiring:
//   - See config.h and module headers for wiring details for LEDs, knobs, and sensors
//
// Usage:
//   - Upload to a compatible Arduino (e.g., Mega 2560)
//   - Adjust knobs or interact with the ultrasound sensor to control effects
//   - Use Serial output for debug info if enabled
//
// File structure:
//   - animation.*: animation logic
//   - palette.*: color palette definitions
//   - knob.h: analog knob mapping
//   - ultrasound.*: distance sensor
//   - debug.h: debug logging utilities
//   - config.h: main configuration
//
// See README.md for more details.

#include "debug.h"
#include "config.h"
#include <FastLED.h>
#include "animation.h"
#include "palette.h"
#include "knob.h"
#include "ultrasound.h"

#ifdef WIFI_ENABLED
#include "wifi_manager.h"
#include "web_server.h"
#endif

CRGB leds[NUM_LEDS];
CRGBPalette256 currentPalette;
TBlendType currentBlending;

// Palette array and index definitions
const TProgmemRGBPalette16* const* gPalettes = PREDEFINED_PALETTES;
const uint8_t gPaletteCount = PREDEFINED_PALETTES_COUNT;
uint8_t gPaletteIndex = PALETTE_INDEX;

static long looper = 0;
long stopper = millis();
bool shouldUpdate = true;

int resolution = RESOLUTION;
int brightness = BRIGHTNESS;
float waveLengthScale = WAVE_LENGTH_SCALE;
float bpm = BPM;

int calculateDelayMillis() {
  int delayMillis = (60000.0) / ((float)NUM_LEDS * bpm * (float)resolution);
  if (delayMillis == 0) {
    dbg::println("Animation configuration reached maximum speed!");
    delayMillis = 1;
  }
  return delayMillis;
}

int delayMillis = calculateDelayMillis();


void setup() {
  dbg::begin();

  pinMode(LED_BUILTIN, OUTPUT);
  digitalWrite(LED_BUILTIN, HIGH);
  dbg::println("\n\nSTART");

  dbg::println("Resolution:  ");
  dbg::println(RESOLUTION);

  dbg::println("delayMillis:  ");
  dbg::println(delayMillis);

#ifdef BRIGHTNESS_KNOB_PIN
  dbg::print("Using brightness knob at pin  ");
  dbg::println(BRIGHTNESS_KNOB_PIN);
#endif

  // Allow setting BPM by a knob
#ifdef BPM_KNOB_PIN
  dbg::print("Using BPM knob at pin  ");
  dbg::println(BPM_KNOB_PIN);
#endif

  // Allow setting wave length scale by a knob
#ifdef WAVE_LENGTH_SCALE_KNOB_PIN
  dbg::print("Using wavelength scale knob at pin  ");
  dbg::println(WAVE_LENGTH_SCALE_KNOB_PIN);
#endif

  ultrasoundSetup();  // If pins are not set, it does not execute anything

  delay(500);

  uint8_t sel = (PALETTE_INDEX < PREDEFINED_PALETTES_COUNT)
                  ? PALETTE_INDEX
                  : 0;
  currentPalette = *(PREDEFINED_PALETTES[sel]);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(LED_STRIP_COLOR_CORRECTION);
  FastLED.setBrightness(brightness);

  // Set currentPalette from palette array and index
  if (gPaletteIndex >= gPaletteCount) gPaletteIndex = 0;
  currentPalette = CRGBPalette16(*gPalettes[gPaletteIndex]);

  dbg::println("Controller setup completed");

  // Initialize WiFi and web server (ESP32 only)
#ifdef ESP32
#ifdef WIFI_ENABLED
  dbg::println("\n=== WiFi & Web Server Initialization ===");
  wifiSetup();

  if (wifiIsConnected()) {
    webServerSetup();
    dbg::println("=== WiFi & Web Server Ready ===\n");
  } else {
    dbg::println("[WARNING] WiFi not connected, web server not started");
  }
#endif
#endif
}

void loop() {
  handleUltrasound();  // If pins are not set, this function does nothing

  long now = millis();
  long expectedLooperDiff = (now - stopper) / delayMillis;

  if (expectedLooperDiff > 0) {
    int missedFrames = expectedLooperDiff - 1;
    looper += expectedLooperDiff;
    shouldUpdate = true;

    if (missedFrames > 0) {
      dbg::print("[ANIMATION] Skipped frames: ");
      dbg::print(missedFrames);
      dbg::print(", ");
      digitalWrite(LED_BUILTIN, HIGH);
    } else {
      digitalWrite(LED_BUILTIN, LOW);
    }
  }

  // Allow setting brightness by a knob
#ifdef BRIGHTNESS_KNOB_PIN
  brightnessByKnob();
#endif

  // Allow setting BPM by a knob
#ifdef BPM_KNOB_PIN
  bpmByKnob();
#endif

  // Allow setting wave length scale by a knob
#ifdef WAVE_LENGTH_SCALE_KNOB_PIN
  waveLengthByKnob();
#endif

  if (shouldUpdate) {
    setLeds();
    stopper = now;
    shouldUpdate = false;
  }
}

void setLeds() {
  FillLEDsFromPaletteColors(looper, resolution, waveLengthScale);
  FastLED.show();
}

void brightnessByKnob() {
#ifdef BRIGHTNESS_KNOB_PIN
  int newBrightness = calculateKnobValueForPin<int>(BRIGHTNESS_KNOB_PIN, BRIGHTNESS_MIN, BRIGHTNESS_MAX, 0, KNOB_5V);
  if (abs(brightness - newBrightness) > BRIGHTNESS_CHANGE_THRESHOLD) {
    if (newBrightness <= BRIGHTNESS_CHANGE_THRESHOLD) { newBrightness = 0; }  // if brightness is below threshold, set it to 0
    dbg::print("[ANIMATION] Brightness changed from ");
    dbg::print(brightness);
    dbg::print(" to ");
    dbg::println(newBrightness);
    brightness = newBrightness;
    FastLED.setBrightness(brightness);
    FastLED.show();
  }
#endif  // BRIGHTNESS_KNOB_PIN
}

void bpmByKnob() {
#ifdef BPM_KNOB_PIN
  float newBPM = calculateKnobValueForPin<float>(BPM_KNOB_PIN, BPM_MIN, BPM_MAX, 0, KNOB_5V);
  if (abs(bpm - newBPM) > BPM_CHANGE_THRESHOLD) {  // add threshold to avoid flickering
    dbg::print("[ANIMATION] BPM changed from ");
    dbg::print(bpm);
    dbg::print(" to ");
    dbg::println(newBPM);
    bpm = newBPM;
    delayMillis = calculateDelayMillis();
  }
#endif  // BPM_KNOB_PIN
}

void waveLengthByKnob() {
#ifdef WAVE_LENGTH_SCALE_KNOB_PIN
  float newWaveLengthScale = calculateKnobValueForPin<float>(WAVE_LENGTH_SCALE_KNOB_PIN, WAVE_LENGTH_SCALE_MIN, WAVE_LENGTH_SCALE_MAX, 0, KNOB_5V);
  if (abs(waveLengthScale - newWaveLengthScale) > WAVE_LENGTH_SCALE_CHANGE_THRESHOLD) {  // add threshold to avoid flickering
    dbg::print("[ANIMATION] Wave length scale changed from ");
    dbg::print(waveLengthScale);
    dbg::print(" to ");
    dbg::println(newWaveLengthScale);
    waveLengthScale = newWaveLengthScale;
    setLeds();
   }
#endif  // WAVE_LENGTH_SCALE_KNOB_PIN
}

void handleUltrasound() {
  // If pins are not set, this function does nothing
  ultrasoundUpdate();

  if (ultrasoundHasReading()) {  // thread blocking check
    float cm = ultrasoundRead_cm();
    // dbg::print("[US] ");
    // dbg::println(cm);

    /* example: map 3-30 cm to brightness 0-255 */
    int newBright = constrain(
      map((int)cm, US_MIN_DISTANCE_CM, US_MAX_DISTANCE_CM, BRIGHTNESS_MAX, BRIGHTNESS_MIN),
      BRIGHTNESS_MIN, BRIGHTNESS_MAX);

    if (abs(newBright - brightness) > BRIGHTNESS_CHANGE_THRESHOLD && cm < (US_MAX_DISTANCE_CM + 10)) {  // ensure that measuers out of the "useful" range are not consumed
      dbg::print("[ANIMATION] Brightness changed from ");
      dbg::print(brightness);
      dbg::print(" to ");
      dbg::println(newBright);
      brightness = newBright;
      FastLED.setBrightness(brightness);
      FastLED.show();
    }
  }
}
