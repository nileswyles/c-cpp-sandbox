#include "etime.h"

#include <time.h>

#define SECONDS_PER_HOUR 3600 // 60 * 60
#define SECONDS_PER_NON_LEAP_YEAR 31536000 // 365 * 60 * 60 * 24

static size_t invocations = 0;

// I think we'll eventually arrive at a full-featured testing framework with mocks and all the other bells and whistles but for now... this works just fine.

// Values were generated at epochconverter.com and confirmed with unixtimestamp.com
//  these tests are reliant on the correctness of epochconverter.com and unixtimestamp.com

// TODO:
//  Validate again using linux cli?
extern uint64_t WylesLibs::Cal::getZonedEpochTime(int8_t offset) {
    uint64_t epoch_seconds;
    invocations++;
    if (invocations == 1) {
        // testGetFormattedTime
        // Date and time (GMT): Monday, February 27, 2023 12:00:00 AM
        epoch_seconds = 1677456000;
    } else if (invocations == 2) {
        // testGetFormattedTime1970
        // Date and time (GMT): Friday, January 2, 1970 12:00:00 AM
        epoch_seconds = 86400;
    } else if (invocations == 3) {
        // testGetFormattedTimeLeapYearBeforeLeapDay
        // Date and time (GMT): Tuesday, February 27, 2024 12:00:00 AM
        epoch_seconds = 1708992000;
    } else if (invocations == 4) {
        // testGetFormattedTimeLeapYearAfterLeapDay
        // Date and time (GMT): Sunday, November 17, 2024 2:57:24 PM
        epoch_seconds = 1731855444;
    } else if (invocations == 5) {
       // testGetFormattedTimeLastDayBeforeLeapYear
        // Date and time (GMT): Sunday, December 31, 2023 11:59:59 PM
        epoch_seconds = 1704067199;
    } else if (invocations == 6) {
        // testGetFormattedTimeFirstDayOfLeapYear
        // Date and time (GMT): Monday, January 1, 2024 12:00:00 AM
        epoch_seconds = 1704067200;
    } else if (invocations == 7) {
        // testGetFormattedTimeLastDayOfLeapYear
        // Date and time (GMT): Tuesday, December 31, 2024 11:59:59 PM
        epoch_seconds = 1735689599;
    } else if (invocations == 8) {
        // testGetFormattedTimeFirstDayAfterLeapYear
        // Date and time (GMT): Wednesday, January 1, 2025 12:00:00 AM
        epoch_seconds = 1735689600;
    }
    return epoch_seconds;
}