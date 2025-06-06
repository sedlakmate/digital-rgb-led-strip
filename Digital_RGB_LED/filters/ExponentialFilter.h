#ifndef EXPONENTIAL_FILTER_H
#define EXPONENTIAL_FILTER_H

/**
 * @brief Applies Exponential Moving Average (EMA) filtering to smooth noisy input signals.
 *
 * This filter is useful for suppressing small flickering or jittery variations
 * in analog or digital signals such as knob readings or distance sensors.
 */
class ExponentialFilter {
  float alpha;       ///< Smoothing factor: 0.0 = heavy smoothing, 1.0 = no smoothing
  float value;       ///< Current smoothed value
  bool initialized;  ///< Tracks if the first input has been received

public:
  /**
     * @brief Construct an ExponentialFilter
     *
     * @param alpha Smoothing factor between 0.0 and 1.0.
     *              Lower values apply heavier smoothing. A typical value is 0.1–0.4.
     */
  ExponentialFilter(float alpha = 0.3)
    : alpha(alpha), value(0.0), initialized(false) {}

  /**
     * @brief Updates the filter with a new input value.
     *
     * @param input New raw value to be filtered.
     * @return Smoothed value after applying the filter.
     */
  float update(float input) {
    if (!initialized) {
      value = input;
      initialized = true;
    } else {
      value = alpha * input + (1 - alpha) * value;
    }
    return value;
  }

  /**
     * @brief Gets the current smoothed value.
     * @return Last computed EMA value.
     */
  float get() const {
    return value;
  }

  /**
     * @brief Resets the filter to uninitialized state.
     */
  void reset() {
    initialized = false;
  }
};

#endif
