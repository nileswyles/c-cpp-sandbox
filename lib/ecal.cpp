#include "ecal.h"
#include "string-format.h"
#include "etime.h"

#include <string>

#include <stdint.h>

// constexpr?

#define SECONDS_PER_HOUR 3600 // 60 * 60
#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_DAY 86400 // 60 * 60 * 24
#define SECONDS_PER_NON_LEAP_YEAR 31536000 // 365 * 60 * 60 * 24

#define MONTHS_IN_YEAR 12
#define SIZE_MONTHS_ARRAY MONTHS_IN_YEAR + 1

static const uint32_t RUNNING_SECONDS_UP_UNTIL_MONTH_NON_LEAP_YEAR[SIZE_MONTHS_ARRAY] = {
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

static const uint32_t RUNNING_SECONDS_UP_UNTIL_MONTH_LEAP_YEAR[SIZE_MONTHS_ARRAY] = {
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

static const char * MONTH_NAME[SIZE_MONTHS_ARRAY] = {
    "",
    "Jan",
    "Feb",
    "Mar",
    "Apr",
    "May",
    "June",
    "July",
    "Aug",
    "Sep",
    "Oct",
    "Nov",
    "Dec"
};

static bool isLeapYear(uint16_t year) {
    return year % 4 == 0 && (year % 100 != 0 || (year % 100 == 0 && year % 400 == 0));
}

extern std::string WylesLibs::Cal::getFormattedDateTime(int8_t offset) {
    uint64_t epoch_seconds = WylesLibs::Cal::getZonedEpochTime(offset);
    
    uint64_t seconds_since_year_start = epoch_seconds;
    uint16_t year = 1970;
    if (epoch_seconds >= SECONDS_PER_NON_LEAP_YEAR) {
        // at least 1971...
        uint64_t gregorian_count = 0; // start from 0 (1970) and count up. (should be okay for ~584942417355.072 years)
        do {
            gregorian_count += SECONDS_PER_NON_LEAP_YEAR;
            if (true == isLeapYear(year)) {
                gregorian_count += SECONDS_PER_DAY; // include feb 29 - 366 days in each leap year
            }
            year++;
        } while (epoch_seconds >= gregorian_count);
        year--;
        seconds_since_year_start = epoch_seconds - (gregorian_count - SECONDS_PER_NON_LEAP_YEAR);
        if (true == isLeapYear(year)) {
            seconds_since_year_start += SECONDS_PER_DAY;
        }
    }
    const uint32_t * running_seconds_up_until_month;
    if (true == isLeapYear(year)) {
        running_seconds_up_until_month = &(RUNNING_SECONDS_UP_UNTIL_MONTH_LEAP_YEAR[0]);
    } else {
        running_seconds_up_until_month = &(RUNNING_SECONDS_UP_UNTIL_MONTH_NON_LEAP_YEAR[0]);
    }

    uint8_t i = 0;
    while (i < SIZE_MONTHS_ARRAY && running_seconds_up_until_month[i] <= seconds_since_year_start) {
        i++;
    } // i == month_index + 1;
    uint8_t month = i;
    uint32_t seconds_up_until_month = running_seconds_up_until_month[i == 0 ? 0 : i - 1];

     // "seconds since year start" will always be less than "seconds up until next month" because of the above... so, I think we can safely add 1 to the day index.
    uint32_t seconds_since_month_start = seconds_since_year_start - seconds_up_until_month;
    uint8_t day = static_cast<uint8_t>(seconds_since_month_start/SECONDS_PER_DAY);

    uint32_t seconds_since_day_start = seconds_since_month_start - day * SECONDS_PER_DAY;
    uint8_t hr = static_cast<uint8_t>(seconds_since_day_start/SECONDS_PER_HOUR);
    uint8_t min = static_cast<uint8_t>((seconds_since_day_start - hr * SECONDS_PER_HOUR) / SECONDS_PER_MINUTE);
    uint8_t sec = static_cast<uint8_t>(seconds_since_day_start - hr * SECONDS_PER_HOUR - min * SECONDS_PER_MINUTE);

    // Month Day Year, 24H:60M:60S
    return WylesLibs::format("{s} {u} {u}, {u}:{u}:{u}", MONTH_NAME[month], day + 1, year, hr, min, sec);
}