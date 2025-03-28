#ifndef WYLESLIBS_ETIME_H
#define WYLESLIBS_ETIME_H

#include <stdint.h>

#define SECONDS_PER_HOUR 3600 // 60 * 60
#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_DAY 86400 // 60 * 60 * 24
#define SECONDS_PER_NON_LEAP_YEAR 31536000 // 365 * 60 * 60 * 24

#define MONTHS_IN_YEAR 12
#define SIZE_MONTHS_ARRAY MONTHS_IN_YEAR + 1

namespace WylesLibs::Cal {


    static constexpr uint32_t RUNNING_SECONDS_UP_UNTIL_MONTH_NON_LEAP_YEAR[SIZE_MONTHS_ARRAY] = {
        0,
        SECONDS_PER_DAY * (31),                                                        // jan (up until feb), index 2)
        SECONDS_PER_DAY * (31 + 28),                                                   // jan + feb (up until march), index 3)
        SECONDS_PER_DAY * (31 + 28 + 31),                                              // april
        SECONDS_PER_DAY * (31 + 28 + 31 + 30),                                         // may
        SECONDS_PER_DAY * (31 + 28 + 31 + 30 + 31),                                    // june
        SECONDS_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30),                               // july
        SECONDS_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31),                          // august
        SECONDS_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31),                     // sept
        SECONDS_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30),                // oct
        SECONDS_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),           // nov
        SECONDS_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30),      // dec
        SECONDS_PER_DAY * (31 + 28 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31), // start of new year.
    };

    static constexpr uint32_t RUNNING_SECONDS_UP_UNTIL_MONTH_LEAP_YEAR[SIZE_MONTHS_ARRAY] = {
        0,
        SECONDS_PER_DAY * (31),                                                        // jan (up until feb), index 2)
        SECONDS_PER_DAY * (31 + 29),                                                   // jan + feb (up until march), index 3)
        SECONDS_PER_DAY * (31 + 29 + 31),                                              // april
        SECONDS_PER_DAY * (31 + 29 + 31 + 30),                                         // may
        SECONDS_PER_DAY * (31 + 29 + 31 + 30 + 31),                                    // june
        SECONDS_PER_DAY * (31 + 29 + 31 + 30 + 31 + 30),                               // july
        SECONDS_PER_DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31),                          // august
        SECONDS_PER_DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31),                     // sept
        SECONDS_PER_DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30),                // oct
        SECONDS_PER_DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31),           // nov
        SECONDS_PER_DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30),      // dec
        SECONDS_PER_DAY * (31 + 29 + 31 + 30 + 31 + 30 + 31 + 31 + 30 + 31 + 30 + 31), // start of new year.
    };

    static bool isLeapYear(uint16_t year) {
        return year % 4 == 0 && (year % 100 != 0 || (year % 100 == 0 && year % 400 == 0));
    }

    extern uint32_t secondsUpUntilMonth(uint8_t month, uint16_t year);
    extern uint16_t getNumLeapYears(uint16_t year);

    extern void setApplicationTimeOffset(int16_t offset);
    extern uint64_t getUTCEpochTime();
    extern uint64_t getZonedEpochTime(int16_t& offset);
};

#endif