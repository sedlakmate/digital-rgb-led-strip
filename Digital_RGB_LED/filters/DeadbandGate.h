#ifndef DEADBAND_GATE_H
#define DEADBAND_GATE_H

/**
 * @brief Holds the last stable value and only allows updates when the input deviates beyond a threshold.
 */
class DeadbandGate {
  float center;
  float tolerance;
  bool initialized;

public:
  /**
     * @param tolerance Acceptable range around the last stable value (e.g. 2.0 for ±2 deadband).
     */
  DeadbandGate(float tolerance = 2.0)
    : center(0.0), tolerance(tolerance), initialized(false) {}

  /**
     * @brief Check whether input deviates enough to be considered a change.
     * @param input New input to check.
     * @return True if the input is outside the deadband.
     */
  bool isOutside(float input) {
    if (!initialized) {
      center = input;
      initialized = true;
      return true;
    }
    float delta = abs(input - center);
    if (delta > tolerance) {
      center = input;
      return true;
    }
    return false;
  }

  /**
     * @brief Get the last accepted center value.
     */
  float getCenter() const {
    return center;
  }

  /**
     * @brief Reset the state.
     */
  void reset() {
    initialized = false;
  }
};

#endif
