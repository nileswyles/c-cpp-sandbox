#include "etime.h"

#include <time.h>

#define SECONDS_PER_HOUR 3600 // 60 * 60

extern uint64_t WylesLibs::Cal::getZonedEpochTime(int8_t offset) {
    // if not initialized assume local time (offset)? where is tz stored should I even care?
    struct timespec ts;
    // TODO: move to cpp file...
    //  make sure to process result here and elsewhere.
    clock_gettime(CLOCK_REALTIME, &ts);
    // gives seconds since epoch (1/1/1970) UTC (no-offset)
    ts.tv_sec += (offset * SECONDS_PER_HOUR);
    return static_cast<uint64_t>(ts.tv_sec);
}