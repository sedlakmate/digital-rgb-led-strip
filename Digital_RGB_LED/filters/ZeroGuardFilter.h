#ifndef ZERO_GUARD_FILTER_H
#define ZERO_GUARD_FILTER_H

/**
 * @brief Filters out unexpected or spurious zero values based on recent history.
 *
 * Especially useful for analog knobs or sliders where 0 can be both valid and erroneous
 * (e.g., circuit disconnection vs intentional input).
 */
class ZeroGuardFilter {
  int* history;   ///< Circular buffer to store recent values
  int size;       ///< Length of history buffer
  int index;      ///< Current write index
  int threshold;  ///< Value above which a sample is considered "high" (i.e., not zero-ish)

public:
  /**
     * @brief Constructs a ZeroGuardFilter.
     *
     * @param size Number of past readings to track (default 5).
     * @param threshold Values above this are considered stable. Values <= threshold are "low".
     */
  ZeroGuardFilter(int size = 5, int threshold = 5)
    : size(size), index(0), threshold(threshold) {
    history = new int[size];
    for (int i = 0; i < size; ++i) history[i] = 0;
  }

  ~ZeroGuardFilter() {
    delete[] history;
  }

  /**
     * @brief Determines whether a given input is a valid value or a faulty zero.
     *
     * @param input The raw value to assess.
     * @return True if the zero is likely intentional or the value is non-zero.
     *         False if the zero appears to be a disconnection or glitch.
     */
  bool isValid(int input) {
    history[index] = input;
    index = (index + 1) % size;

    if (input != 0) return true;

    int highCount = 0;
    for (int i = 0; i < size; ++i)
      if (history[i] > threshold) highCount++;

    return highCount < (size / 2);  // mostly low = likely valid zero
  }
};

#endif
