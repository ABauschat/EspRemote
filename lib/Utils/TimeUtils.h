#ifndef TIME_UTILS_H
#define TIME_UTILS_H

#include <Arduino.h>

namespace NuggetsInc {

using msec32 = uint32_t;

static inline msec32 now_ms() { 
    return (msec32)millis(); 
}

static inline bool has_elapsed(msec32 start_ms, msec32 timeout_ms) {
    return (msec32)(now_ms() - start_ms) >= timeout_ms;
}

static inline msec32 elapsed_ms(msec32 start_ms) {
    return (msec32)(now_ms() - start_ms);
}

static inline bool within_window(msec32 timestamp, msec32 window_ms) {
    return elapsed_ms(timestamp) < window_ms;
}

} // namespace NuggetsInc

#endif // TIME_UTILS_H
