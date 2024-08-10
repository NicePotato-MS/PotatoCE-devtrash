#include <time.h>
#include <limits.h>
#include "clock.h"

clock_t curClock;

clock_t calculateElapsedTime(clock_t oldTime) {
    if (curClock < oldTime) {
        // Handle wrapping (untested)
        return (LONG_MAX-oldTime)+curClock+1;
    } else {
        // No wrapping
        return curClock-oldTime;
    }
}