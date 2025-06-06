#ifndef INPUT_PIPELINE_H
#define INPUT_PIPELINE_H

#include "ZeroGuardFilter.h"
#include "ClampedFilter.h"
#include "AdaptiveSmoother.h"
#include "DeadbandGate.h"

/**
 * @brief Composable pipeline that validates and (optionally) filters input signals.
 */
class InputPipeline {
  ZeroGuardFilter* zeroGuard;
  ClampedFilter* clampedGuard;
  AdaptiveSmoother* smoother;
  DeadbandGate* deadband;
  float lastValue;


public:
  InputPipeline()
    : zeroGuard(nullptr), clampedGuard(nullptr), smoother(nullptr), lastValue(0.0f) {}

  void useZeroGuard(int size, int threshold) {
    if (zeroGuard) delete zeroGuard;
    zeroGuard = new ZeroGuardFilter(size, threshold);
  }

  void useDeadband(float tolerance) {
    if (deadband) delete deadband;
    deadband = new DeadbandGate(tolerance);
  }

  void useClampedGuard(int minValue, int maxValue) {
    if (clampedGuard) delete clampedGuard;
    clampedGuard = new ClampedFilter(minValue, maxValue);
  }

  void useAdaptiveSmoother(float alpha, float jitterThreshold) {
    if (smoother) delete smoother;
    smoother = new AdaptiveSmoother(alpha, jitterThreshold);
  }

  bool update(int input) {
    if (zeroGuard && !zeroGuard->isValid(input)) return false;
    if (clampedGuard && !clampedGuard->isValid(input)) return false;

    if (deadband && !deadband->isOutside(input)) {
      return false;  // input within deadband → hold current value
    }

    if (smoother) {
      lastValue = smoother->update(input);
    } else {
      lastValue = input;
    }

    return true;
  }

  float get() const {
    return lastValue;
  }

  void reset() {
    if (smoother) smoother->reset();
    lastValue = 0.0f;
  }

  ~InputPipeline() {
    if (zeroGuard) delete zeroGuard;
    if (clampedGuard) delete clampedGuard;
    if (smoother) delete smoother;
    if (deadband) delete deadband;
  }
};

#endif
