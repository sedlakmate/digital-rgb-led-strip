#include "animation.h"
#include "config.h"
#include <FastLED.h>
#include <ctype.h>
#include "palette.h"

// Global virtual LED buffer and its length.
CRGB* gVirtualLeds = nullptr;
long gVirtualLedCount = 0;

// Compute how many physical sections are actively rendered before mirroring.
static long normalizedFoldCount() {
  long folds = ANIMATION_PARTS;
  if (folds < 1) {
    folds = 1;
  }
  return folds;
}

static long getIndependentLedCount() {
  long folds = normalizedFoldCount();
  long unique = (NUM_LEDS + folds - 1) / folds;  // ceil division keeps all LEDs covered
  if (unique < 1) {
    unique = 1;
  }
  return unique;
}

// Map a logical LED index into the canonical (independent) section, mirroring derived folds.
static long canonicalIndexForLed(long ledIndex, long canonicalLen);

static bool equalsIgnoreCase(const char* lhs, const char* rhs) {
  if (!lhs || !rhs) {
    return false;
  }
  while (*lhs && *rhs) {
    char lc = (char)tolower(*lhs++);
    char rc = (char)tolower(*rhs++);
    if (lc != rc) {
      return false;
    }
  }
  return (*lhs == '\0' && *rhs == '\0');
}

static bool partsTypeIsFolded() {
  return equalsIgnoreCase(ANIMATION_PARTS_TYPE, "FOLDED");
}

static long canonicalIndexForLed(long ledIndex, long canonicalLen) {
  if (canonicalLen <= 0) {
    return 0;
  }
  long folds = normalizedFoldCount();
  if (folds <= 1) {
    return ledIndex;
  }
  long section = ledIndex / canonicalLen;
  if (section >= folds) {
    section = folds - 1;
  }
  long sectionOffset = ledIndex % canonicalLen;
  if (!partsTypeIsFolded()) {
    return sectionOffset;
  }
  bool mirrorSection = (section % 2 == 1);
  if (!mirrorSection) {
    return sectionOffset;
  }
  long mirrored = canonicalLen - 1 - sectionOffset;
  if (mirrored < 0) {
    mirrored = 0;
  }
  return mirrored;
}

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

  long effectiveLedCount = getIndependentLedCount();
  if (effectiveLedCount < 1) {
    effectiveLedCount = NUM_LEDS;
  }

  // Determine virtual pattern length based on the independent LED count so
  // animations shrink with ANIMATION_PARTS and then mirror/copy across sections.
  float rawLength = (float)effectiveLedCount * waveLengthScale;
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

  const long independentLedCount = getIndependentLedCount();
  const long foldCount = normalizedFoldCount();
  const bool isFoldedMode = partsTypeIsFolded();
  const bool invertCanonicalForInward = (isFoldedMode && foldCount > 1 && independentLedCount > 0 && !ANIMATION_REVERSED);

  for (int physicalIndex = 0; physicalIndex < NUM_LEDS; physicalIndex++) {
    long logicalIndex = ANIMATION_REVERSED ? (NUM_LEDS - physicalIndex - 1) : physicalIndex;
    long canonicalIndex = canonicalIndexForLed(logicalIndex, independentLedCount);
    if (invertCanonicalForInward) {
      canonicalIndex = (independentLedCount - 1) - canonicalIndex;
    }
    long virtualIndex = (wrappedBaseShift + canonicalIndex) % gVirtualLedCount;

    CRGB color;
    if (resolution == 1) {
      color = gVirtualLeds[virtualIndex];
    } else {
      long nextVirtualIndex = (virtualIndex + 1) % gVirtualLedCount;
      color = blend(gVirtualLeds[virtualIndex], gVirtualLeds[nextVirtualIndex], blendFactor);
    }

    leds[physicalIndex] = color;
  }
}
