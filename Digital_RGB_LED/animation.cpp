#include "animation.h"
#include "config.h"
#include <FastLED.h>
#include "palette.h"

void FillLEDsFromPaletteColors(long colorShift, int resolution, float waveLengthScale) {
  for (int i = 0; i < NUM_LEDS; i++) {
    int index = i;
    if (REVERSED) {
      index = NUM_LEDS - i - 1;
    }

    // TODO: Fix reversed
    if (index > NUM_LEDS / 2) {
      leds[index] = leds[NUM_LEDS-index];
      continue;
    }

    long colorIndex = ((long)i * (long)256) / waveLengthScale / ((long)NUM_LEDS) + colorShift / (long)resolution;

    if (resolution == 1) {
      leds[index] = ColorFromPalette(currentPalette, colorIndex + colorShift, 255, currentBlending);  // brightness is set by `FastLED.setBrightness()` and not by "255"
    } else {
      // Apply smoothing
      leds[index] = blend(
        ColorFromPalette(currentPalette, colorIndex, 255, currentBlending),      // brightness is set by `FastLED.setBrightness()` and not by "255"
        ColorFromPalette(currentPalette, colorIndex + 1, 255, currentBlending),  // brightness is set by `FastLED.setBrightness()` and not by "255"
        (float)(colorShift % resolution) * 255.0 / (float)resolution);           // here "255" is the [blend factor](https://github.com/FastLED/FastLED/blob/9696074c3a267ed4237a3caa63a2fd079a48c19d/src/crgb.cpp#L54)
    }
  }
}
