#ifndef ANIMATION_H
#define ANIMATION_H

#include <FastLED.h>

// Precomputed virtual LED strip state.
// Length depends on NUM_LEDS, WAVE_LENGTH_SCALE and RESOLUTION.
extern CRGB* gVirtualLeds;
extern long gVirtualLedCount;

// Rebuild the virtual LED buffer according to current
// WAVE_LENGTH_SCALE and RESOLUTION. Safe to call whenever these change.
void RebuildVirtualLeds(float waveLengthScale, int resolution);

// Render function: maps the virtual LED state to the physical strip by
// sliding a window over gVirtualLeds using the given colorShift.
void FillLEDsFromPaletteColors(long colorShift, int resolution, float waveLengthScale);

#endif  // ANIMATION_H
