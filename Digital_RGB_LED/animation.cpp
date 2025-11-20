#include "animation.h"
#include "config.h"
#include <FastLED.h>
#include "palette.h"

// Global virtual LED buffer and its length.
CRGB* gVirtualLeds = nullptr;
long gVirtualLedCount = 0;

// Helper to free any existing virtual buffer.
static void freeVirtualLeds() {
  if (gVirtualLeds != nullptr) {
    free(gVirtualLeds);
    gVirtualLeds = nullptr;
    gVirtualLedCount = 0;
  }
}

// Rebuilds the virtual LED array based on the provided waveLengthScale and resolution.
//
// Semantics:
//   - The base virtual pattern length is round(NUM_LEDS * waveLengthScale).
//   - This length may be smaller or larger than NUM_LEDS.
//     * If smaller (WAVE_LENGTH_SCALE < 1), the pattern is tiled (repeated)
//       across the physical LEDs.
//     * If larger (WAVE_LENGTH_SCALE > 1), the physical LEDs see only a
//       sliding window into the longer virtual wave.
//   - The pattern is generated from the current palette in that virtual space.
//   - RESOLUTION is used later in FillLEDsFromPaletteColors for phase
//     interpolation; it does not influence buffer size.
void RebuildVirtualLeds(float waveLengthScale, int resolution) {
  // Guard against invalid parameters.
  if (waveLengthScale <= 0.0f) {
    waveLengthScale = 1.0f;
  }
  if (resolution <= 0) {
    resolution = 1;
  }

  // Determine virtual pattern length based purely on scale. Allow patterns
  // shorter than NUM_LEDS so they can be repeated along the strip.
  float rawLength = (float)NUM_LEDS * waveLengthScale;
  long patternSize = (long)(rawLength + 0.5f);
  if (patternSize < 1) patternSize = 1;  // at least one virtual pixel

  // Optional safety cap; adjust if you know your MCU can handle more.
  const long MAX_VIRTUAL_LEDS = 3000;  // ~9 KB for CRGB on 8-bit AVR
  if (patternSize > MAX_VIRTUAL_LEDS) {
    patternSize = MAX_VIRTUAL_LEDS;
  }

  // If size didn't change, we just regenerate in-place to reflect palette
  // or configuration changes.
  bool sizeChanged = (patternSize != gVirtualLedCount);

  if (sizeChanged) {
    freeVirtualLeds();
    gVirtualLeds = (CRGB*)malloc(sizeof(CRGB) * patternSize);
    if (!gVirtualLeds) {
      // Allocation failed; fall back to no virtual pattern.
      gVirtualLedCount = 0;
      return;
    }
    gVirtualLedCount = patternSize;
  }

  // Populate the virtual pattern from the current palette.
  // We map the virtual index linearly into the 0-255 palette index space.
  for (long i = 0; i < gVirtualLedCount; ++i) {
    uint8_t paletteIndex = (uint8_t)((i * 256L) / gVirtualLedCount);
    gVirtualLeds[i] = ColorFromPalette(currentPalette, paletteIndex, 255, currentBlending);
  }
}

void FillLEDsFromPaletteColors(long colorShift, int resolution, float waveLengthScale) {
  // Ensure we have a valid virtual pattern that matches the current
  // waveLengthScale/resolution configuration.
  if (gVirtualLeds == nullptr || gVirtualLedCount <= 0) {
    RebuildVirtualLeds(waveLengthScale, resolution);
  }

  if (gVirtualLeds == nullptr || gVirtualLedCount <= 0) {
    // Still nothing usable; as a last resort, write black.
    for (int i = 0; i < NUM_LEDS; ++i) {
      leds[i] = CRGB::Black;
    }
    return;
  }

  if (resolution <= 0) resolution = 1;

  // Interpret colorShift as the global frame counter. We derive an integer
  // base index and a sub-frame phase from it so that:
  //   - Over NUM_LEDS * resolution frames (one beat), the base index advances
  //     by exactly NUM_LEDS steps, independent of resolution.
  //   - Increasing resolution only adds more interpolation phases between
  //     neighbouring virtual pixels, without speeding up the pattern.
  long frame = colorShift;

  long integerStep = frame / resolution;          // base start index
  uint8_t phase = (uint8_t)(frame % resolution);  // 0 .. (resolution-1)
  uint8_t blendFactor = (uint8_t)((255L * phase) / resolution);

  // Wrap base index into [0, gVirtualLedCount).
  long wrappedBaseShift = integerStep % gVirtualLedCount;
  if (wrappedBaseShift < 0) wrappedBaseShift += gVirtualLedCount;

  for (int i = 0; i < NUM_LEDS; i++) {
    int index = i;
    if (REVERSED) {
      index = NUM_LEDS - i - 1;
    }

    long virtualIndex = (wrappedBaseShift + i) % gVirtualLedCount;

    if (resolution == 1) {
      leds[index] = gVirtualLeds[virtualIndex];
    } else {
      long nextVirtualIndex = (virtualIndex + 1) % gVirtualLedCount;
      leds[index] = blend(gVirtualLeds[virtualIndex], gVirtualLeds[nextVirtualIndex], blendFactor);
    }
  }
}
