// This file allows you to locally override configuration parameters from config.h.
// Only include the settings you want to change. Do not track this file in version control.

#ifndef CONFIG_OVERRIDE_H
#define CONFIG_OVERRIDE_H

/* ─────────── Debug flag ─────────────── */
#undef DEBUG
#define DEBUG 1

/* ──────────── Hardware pins ──────────── */
#define LED_PIN 50  // Arduino digital pin
// #define US_TRIG_PIN 7
// #define US_ECHO_PIN 8
// #define BRIGHTNESS_KNOB_PIN A0
// #define BPM_KNOB_PIN A0
// #define WAVE_LENGTH_SCALE_KNOB_PIN A0

/* ────────── LED configuration ────────── */
#define NUM_LEDS 301  // total LEDs on the strip


/* ─────────── Animation defaults ─────── */
#define BRIGHTNESS 255  // 0-255, initial brightness
#define BPM 2
#define WAVE_LENGTH_SCALE 0.1
#define RESOLUTION 10
#define REVERSED false
#define PALETTE_INDEX 6  // 0-based index into PREDEFINED_PALETTES

/* ────────── Animation limits ────────── */
// #define BPM_MIN 0.01
// #define BPM_MAX 15.0
#define WAVE_LENGTH_SCALE_MIN 0.01
#define WAVE_LENGTH_SCALE_MAX 5.0


#endif  // CONFIG_OVERRIDE_H
