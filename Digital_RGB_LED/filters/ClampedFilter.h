#ifndef CLAMPED_FILTER_H
#define CLAMPED_FILTER_H

/**
 * @brief Rejects readings that fall outside an acceptable range.
 *
 * Useful for sensors like ultrasonic rangefinders, where extreme values
 * (near zero or max range) often indicate faulty measurements.
 */
class ClampedFilter {
  int minValid;  ///< Minimum accepted value (inclusive)
  int maxValid;  ///< Maximum accepted value (inclusive)
  int lastGood;  ///< Last valid value passed through

public:
  /**
     * @brief Constructs a ClampedFilter.
     *
     * @param minValid Minimum allowed value. Readings below this will be rejected.
     * @param maxValid Maximum allowed value. Readings above this will be rejected.
     */
  ClampedFilter(int minValid, int maxValid)
    : minValid(minValid), maxValid(maxValid), lastGood(0) {}

  /**
     * @brief Checks whether the input is within the valid range.
     *
     * @param input The raw value to check.
     * @return True if valid. False if outside allowed range.
     *         If valid, the value is stored internally as the latest good value.
     */
  bool isValid(int input) {
    if (input < minValid || input > maxValid) return false;
    lastGood = input;
    return true;
  }

  /**
     * @brief Gets the last value that was accepted as valid.
     *
     * @return Last valid input.
     */
  int getLastGood() const {
    return lastGood;
  }
};

#endif
