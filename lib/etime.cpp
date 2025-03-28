#include "etime.h"
#include "string_format.h"
#include "datastructures/array.h"

#include <string>
#include <stdexcept>

#if defined(_MSC_VER)
#include <windows.h>
#else
#include <time.h>
#endif

// make sure global logger level is initialized...
#ifndef GLOBAL_LOGGER_LEVEL
#define GLOBAL_LOGGER_LEVEL 0
#endif

// if per module logger level not defined, set to global...
#ifndef LOGGER_LEVEL_ETIME
#define LOGGER_LEVEL_ETIME GLOBAL_LOGGER_LEVEL
#endif

// enable toggle...
#ifndef LOGGER_ETIME
#define LOGGER_ETIME 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_ETIME

#undef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_LEVEL_ETIME
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Cal;

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

static int8_t APPLICATION_TIME_OFFSET = 0; // UTC is default

extern uint32_t WylesLibs::Cal::secondsUpUntilMonth(uint8_t month, uint16_t year) {
    const uint32_t * running_seconds_up_until_month;
    if (true == isLeapYear(year)) {
        running_seconds_up_until_month = &(RUNNING_SECONDS_UP_UNTIL_MONTH_LEAP_YEAR[0]);
    } else {
        running_seconds_up_until_month = &(RUNNING_SECONDS_UP_UNTIL_MONTH_NON_LEAP_YEAR[0]);
    }
    return running_seconds_up_until_month[month - 1];
}

extern uint16_t WylesLibs::Cal::getNumLeapYears(uint16_t year) {
    uint16_t num_leap_years = 0;
    uint16_t year_cursor = 1972;
    while (year_cursor < year) {
        loggerPrintf(LOGGER_DEBUG, "year_cursor: %u, year: %u\n", year_cursor, year);
        if (isLeapYear(year_cursor)) { 
            num_leap_years++;
        }
        year_cursor += 4;
    }
    loggerPrintf(LOGGER_DEBUG, "num leap years: %u\n", num_leap_years);
    return num_leap_years;
}

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
    #if defined(_MSC_VER)
    SYSTEMTIME st;
    GetSystemTime(&st);

    uint64_t seconds = (st.wYear - 1970) * SECONDS_PER_NON_LEAP_YEAR;
    seconds += secondsUpUntilMonth(st.wMonth, st.wYear);
    seconds += st.wHour * SECONDS_PER_HOUR;
    seconds += st.wMinute * SECONDS_PER_MINUTE;
    seconds += st.wSecond;

    return seconds;
    #else
    struct timespec ts;
    // time function appears to do the same thing? "seconds from the epoch"... if nothing else, this is more expressive so let's use this.
    //  time might use CLOCK_REALTIME_COARSE - whatever that is.
    clock_gettime(CLOCK_REALTIME, &ts);
    if (false == OFFSETS.contains(offset)) {
        offset = APPLICATION_TIME_OFFSET;
    }
    ts.tv_sec += (offset * SECONDS_PER_HOUR);
    return static_cast<uint64_t>(ts.tv_sec);
    #endif
}