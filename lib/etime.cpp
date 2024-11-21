#include "etime.h"
#include "string-format.h"
#include "datastructures/array.h"

#include <string>
#include <stdexcept>

#include <time.h>

using namespace WylesLibs;

#define SECONDS_PER_HOUR 3600 // 60 * 60

static int8_t APPLICATION_TIME_OFFSET = 0; // UTC is default

static Array<int16_t> OFFSETS{
    -1200,
    -1100,
    -1000,
    -900,
    -800,
    -700,
    -600,
    -500,
    -400,
    -300,
    -200,
    -100,
    0,
    100,
    200,
    300,
    330,
    400,
    430,
    500,
    530,
    545,
    600,
    700,
    800,
    900,
    930,
    1000,
    1030,
    1100,
    1200,
    1245,
    1300,
    1345,
    1400
};

extern void WylesLibs::Cal::setApplicationTimeOffset(int16_t offset) {
    if (false == OFFSETS.contains(offset)) {
        throw std::runtime_error(WylesLibs::format("Invalid offset provided: {d}", offset));
    } else {
        APPLICATION_TIME_OFFSET = offset;
    }
}

extern uint64_t WylesLibs::Cal::getUTCEpochTime() {
    int16_t offset = 0;
    return WylesLibs::Cal::getZonedEpochTime(offset);
}

extern uint64_t WylesLibs::Cal::getZonedEpochTime(int16_t& offset) {
    struct timespec ts;
    // time function appears to do the same thing? "seconds from the epoch"... if nothing else, this is more expressive so let's use this.
    //  time might use CLOCK_REALTIME_COARSE - whatever that is.
    clock_gettime(CLOCK_REALTIME, &ts);
    if (false == OFFSETS.contains(offset)) {
        offset = APPLICATION_TIME_OFFSET;
    }
    ts.tv_sec += (offset * SECONDS_PER_HOUR);
    return static_cast<uint64_t>(ts.tv_sec);
}