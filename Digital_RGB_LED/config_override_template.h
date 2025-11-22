// Example local override configuration for Digital_RGB_LED
// Only include the settings you want to change. Do not track this file in version control.
//
// To use: copy this file to config_override.h and edit as needed.

#ifndef CONFIG_OVERRIDE_H
#define CONFIG_OVERRIDE_H

// ─────────── Debug flag ───────────────
#undef DEBUG
#define DEBUG 0  // Set to 1 to enable debug output

/* ──────────── Hardware pins ──────────── */
// Define only the pins you are actually using. If a pin is not defined, the feature is disabled.
#define US_TRIG_PIN 7
#define US_ECHO_PIN 8
#define BRIGHTNESS_KNOB_PIN A0
#define BPM_KNOB_PIN A0
#define WAVE_LENGTH_SCALE_KNOB_PIN A0

// ────────── Animation parameters (optional overrides) ──────────
#define BRIGHTNESS 220  // Initial brightness (0-255)
#define BPM 4.0         // Initial beats per minute
#define WAVE_LENGTH_SCALE 1.5
#define RESOLUTION 3
#define ANIMATION_REVERSED false
#define ANIMATION_PARTS 1
#define ANIMATION_PARTS_TYPE "FOLDED"

// Add more overrides as needed, following the pattern above.
// Do not include settings you do not wish to override.

#endif  // CONFIG_OVERRIDE_H
