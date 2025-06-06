#ifndef ADAPTIVE_SMOOTHER_H
#define ADAPTIVE_SMOOTHER_H

/**
 * @brief Applies exponential smoothing only when input variation is below a defined threshold.
 *
 * Useful for scenarios where occasional large input jumps are meaningful and should not be smoothed,
 * but small oscillations should be stabilized.
 */
class AdaptiveSmoother {
  float alpha;            ///< Smoothing factor
  float lastValue;        ///< Last smoothed or raw value
  bool initialized;       ///< Tracks if the smoother is initialized
  float jitterThreshold;  ///< Maximum delta to apply smoothing

public:
  /**
     * @brief Constructs an adaptive smoother.
     *
     * @param alpha Smoothing factor for EMA (e.g., 0.2).
     * @param jitterThreshold Values within this delta will be smoothed; otherwise replaced directly.
     */
  AdaptiveSmoother(float alpha = 0.3, float jitterThreshold = 3.0)
    : alpha(alpha), lastValue(0.0), initialized(false), jitterThreshold(jitterThreshold) {}

  /**
     * @brief Updates the smoother with a new input.
     *
     * @param input Raw input value.
     * @return New smoothed or direct value based on input fluctuation.
     */
  float update(float input) {
    if (!initialized) {
      lastValue = input;
      initialized = true;
    } else {
      float delta = abs(input - lastValue);
      if (delta <= jitterThreshold) {
        lastValue = alpha * input + (1 - alpha) * lastValue;  // smooth small change
      } else {
        lastValue = input;  // accept jump directly
      }
    }
    return lastValue;
  }

  /**
     * @brief Returns the current internal value.
     */
  float get() const {
    return lastValue;
  }

  /**
     * @brief Resets internal state.
     */
  void reset() {
    initialized = false;
  }
};

#endif
