#include "etime.h"
#include "string-format.h"

#include <string>
#include <stdexcept>

#include <time.h>

#define SECONDS_PER_HOUR 3600 // 60 * 60

static int8_t APPLICATION_TIME_OFFSET = 0; // UTC is default

extern void WylesLibs::Cal::setApplicationTimeOffset(int8_t offset) {
    if (offset < -12 || 12 < offset) {
        std::string msg = WylesLibs::format("Invalid offset provided: {d}", offset);
        throw std::runtime_error(msg);
    } else {
        APPLICATION_TIME_OFFSET = offset;
    }
}

extern uint64_t WylesLibs::Cal::getUTCEpochTime() {
    return WylesLibs::Cal::getZonedEpochTime(0);
}

extern uint64_t WylesLibs::Cal::getZonedEpochTime(int8_t offset) {
    struct timespec ts;
    // time function appears to do the same thing? "seconds from the epoch"... if nothing else, this is more expressive so let's use this.
    //  time might use CLOCK_REALTIME_COARSE - whatever that is.
    clock_gettime(CLOCK_REALTIME, &ts);
    if (offset < -12 || 12 < offset) {
        offset = APPLICATION_TIME_OFFSET;
    }
    ts.tv_sec += (offset * SECONDS_PER_HOUR);
    return static_cast<uint64_t>(ts.tv_sec);
}