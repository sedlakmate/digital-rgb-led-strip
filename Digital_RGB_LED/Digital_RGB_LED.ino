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
#include "filters/InputPipeline.h"

CRGB leds[NUM_LEDS];
CRGBPalette256 currentPalette;
TBlendType currentBlending;

#ifdef WAVE_LENGTH_SCALE_KNOB_PIN
InputPipeline waveLengthPipeline;
#endif

// Palette array and index definitions
const TProgmemRGBPalette16* const* gPalettes = PREDEFINED_PALETTES;  // matches extern in config.h
const uint8_t gPaletteCount = PREDEFINED_PALETTES_COUNT;
uint8_t gPaletteIndex = PALETTE_INDEX;

// looper is the global colorShift used to index into the virtual pattern.
// It is incremented by 1 for each physical frame; RESOLUTION is handled as
// a separate multiplier on the frame rate rather than baked into looper.
static long looper = 0;
long stopper = millis();
bool shouldUpdate = true;

int resolution = RESOLUTION;
int brightness = BRIGHTNESS;
float waveLengthScale = WAVE_LENGTH_SCALE;
float bpm = BPM;

// Floating-point map function
float mapf(float x, float in_min, float in_max, float out_min, float out_max) {
  return (x - in_min) * (out_max - out_min) / (in_max - in_min) + out_min;
}

// Calculate the delay between frames so that:
//   - One "beat" (as defined by BPM) is the time required to step through
//     NUM_LEDS distinct start indices in the virtual pattern.
//   - If RESOLUTION > 1, we refresh RESOLUTION times more often, i.e. there
//     are NUM_LEDS * RESOLUTION frames per beat.
int calculateDelayMillis() {
  // Frames per beat = NUM_LEDS * resolution
  float framesPerBeat = (float)NUM_LEDS * (float)resolution;
  // Beats per second = bpm / 60
  float beatsPerSecond = bpm / 60.0f;
  // Frames per second = framesPerBeat * beatsPerSecond
  float framesPerSecond = framesPerBeat * beatsPerSecond;

  if (framesPerSecond <= 0.0f) {
    dbg::println("Animation configuration reached maximum speed or invalid BPM!");
    return 1;
  }

  int delayMillis = (int)(1000.0f / framesPerSecond);
  if (delayMillis < 1) {
    dbg::println("Animation configuration reached maximum speed or invalid BPM!");
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
  waveLengthPipeline.useZeroGuard(6, 10);        // History length = 6, threshold = 10
  waveLengthPipeline.useDeadband(7.0);           // ±2 analog units = no update
  waveLengthPipeline.useAdaptiveSmoother(0.1, 250.0);  // EMA with alpha = 0.1
#endif

  ultrasoundSetup();  // If pins are not set, it does not execute anything

  delay(300);

  uint8_t sel = (PALETTE_INDEX < PREDEFINED_PALETTES_COUNT)
                  ? PALETTE_INDEX
                  : 0;
  currentPalette = *(PREDEFINED_PALETTES[sel]);

  FastLED.addLeds<LED_TYPE, LED_PIN, COLOR_ORDER>(leds, NUM_LEDS).setCorrection(LED_STRIP_COLOR_CORRECTION);
  FastLED.setBrightness(brightness);

  // Set currentPalette from palette array and index
  if (gPaletteIndex >= gPaletteCount) gPaletteIndex = 0;
  currentPalette = CRGBPalette16(*gPalettes[gPaletteIndex]);

  // Build initial virtual LED buffer based on starting waveLengthScale/resolution
  RebuildVirtualLeds(waveLengthScale, resolution);

  dbg::println("Controller setup completed");
}

void loop() {
  handleUltrasound();  // If pins are not set, this function does nothing

  long now = millis();
  long elapsed = now - stopper;

  if (elapsed >= delayMillis) {
    long expectedFrames = elapsed / delayMillis;
    int missedFrames = (int)expectedFrames - 1;

    // Advance looper by the number of frames we conceptually rendered.
    looper += expectedFrames;
    shouldUpdate = true;

    if (missedFrames > 0) {
      // dbg::print("[ANIMATION] Skipped frames: ");
      // dbg::print(missedFrames);
      // dbg::print(", ");
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
  // Always render from the precomputed virtual buffer.
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
  int raw = analogRead(WAVE_LENGTH_SCALE_KNOB_PIN);

  if (waveLengthPipeline.update(raw)) {
    float filtered = waveLengthPipeline.get();

    float newWaveLengthScale = mapf(filtered, 0, KNOB_5V, WAVE_LENGTH_SCALE_MIN, WAVE_LENGTH_SCALE_MAX);

    if (abs(waveLengthScale - newWaveLengthScale) > WAVE_LENGTH_SCALE_CHANGE_THRESHOLD) {
      dbg::println("");
      dbg::print("[ANIMATION] Wave length scale changed from ");
      dbg::print(waveLengthScale);
      dbg::print(" to ");
      dbg::println(newWaveLengthScale);
      waveLengthScale = newWaveLengthScale;
      // Rebuild virtual buffer to reflect new wave length scale.
      RebuildVirtualLeds(waveLengthScale, resolution);
      setLeds();
    }
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
    dbg::println("STALE FUNCTION: `map`. Use `mapf` instead.");
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
