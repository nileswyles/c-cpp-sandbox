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
#define SIZE_RUNNING_MONTHS_ARRAY MONTHS_IN_YEAR + 1
static const uint32_t runningSecondsUpUntilMonthNonLeapYear[MONTHS_IN_YEAR] = {
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

static const uint32_t runningSecondsUpUntilMonthLeapYear[MONTHS_IN_YEAR] = {
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

// TODO: speed it up further by precalculating running seconds instead of days...
static const char * monthName[SIZE_RUNNING_MONTHS_ARRAY] = {
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

// static uint16_t getNumLeapYears(uint64_t time) {
//     uint16_t num_leap_years = 0;
    
//     uint16_t greg_count = SECONDS_PER_NON_LEAP_YEAR * 2; // start from 1972 and count up.
//     uint16_t year_count = 1972;
//     while (time > greg_count) {
//         greg_count += SECONDS_PER_NON_LEAP_YEAR;
//         if (isLeapYear(year_count)) {
//             time -= SECONDS_PER_DAY;
//             num_leap_years++;
//         }
//         year_count++;
//     }
    // can simplify calculation of seconds to year start by calculating that here...
    // it's greg_count[i - 1]; and time - greg_count[i - 1] == seconds since year start...
    //  hmm... so, only care about leap years to add the extra day...
    //  lol, ddd.
    //
    //  okay, so might want to branch...
    //  the goal is no longer to calculate num_leap_years and return num seconds up until year...
    //  the remainder of the algorithm remains the same... you can get this counting up or down, doesn't matter.. but need to count up if following greg rules for leap year?

//     return num_leap_years, greg_count;
// }

static uint16_t getNumLeapYears(uint64_t time) {
    uint16_t num_leap_years = 0;
    
    uint16_t year_count = 0;
    while (time >= SECONDS_PER_NON_LEAP_YEAR * 2) { // we can stop at 1972, the first leap year after 1970
        time -= SECONDS_PER_NON_LEAP_YEAR;
        if (year_count != 0 && isLeapYear(year_count)) {
            time -= SECONDS_PER_DAY;
            num_leap_years++;
        }
        year_count++;
    }
    return num_leap_years;
}

extern std::string WylesLibs::Cal::getFormattedDateTime(int8_t offset) {
    uint64_t epoch_seconds = WylesLibs::Cal::getZonedEpochTime(offset);
    // Month Day Year, 24H:60M:60S

    // year - 1970 = year_diff
    //  year_diff_seconds = (year_diff - number of leap years) * seconds_per_year + (number of leap years * seconds_per_leap_year)
    //  or 
    //  year_diff_seconds = year_diff * seconds_per_year + (number of leap years * seconds_per_day); // right? because we add an extra day?

    // year_diff_seconds == ts.tv_sec

    // leap_adj_seconds = ts.tv_sec - (num_leap_years * seconds_per_day);

    //  year = (uint16_t)leap_adj_seconds/seconds_per_non_leap_year + 1970 // leap adjusted year...

    //      seconds_since_year_start = leap_adj_seconds - year * seconds_per_non_leap_year;
    //  month = seconds_since_year_start / seconds_up_until_non_leap_month; 
    //  day = leap_adj_seconds - year * seconds_per_non_leap_year - month * seconds_up_until_non_leapyear_month / seconds_per_day

    //  if (leap_year) {
    //      month = seconds_since_year_start / seconds_up_until_leap_month;
    //      day = leap_adj_seconds - year * seconds_per_non_leap_year - month * seconds_up_until_leapyear_month / seconds_per_day;
    //  }
    //      
    //  if (leap_year and month > 2) {
    //  }
    uint16_t num_leap_years = getNumLeapYears(epoch_seconds);
    printf("NUMBER OF LEAP YEARS: %u\n", num_leap_years);
    uint64_t leap_adj_seconds = epoch_seconds - (num_leap_years * SECONDS_PER_DAY); // this should make it so the following calculation is off by no more than 1 day.
    // let's see...
    //  Jan 1, 2020
    //  Dec 31, 2019

    // and
    //  Dec 31, 2020
    //  Jan 1, 2021

    // TODO: cast vs floor...
    uint16_t year_since_1970 = static_cast<uint16_t>(leap_adj_seconds/SECONDS_PER_NON_LEAP_YEAR);
    uint16_t year = year_since_1970 + 1970;

    // leap_adj_seconds is epoch seconds minus leap_days up until that year...
    //      so, leap_adj_seconds is effectively (in example above)... epoch_seconds - 12 days. (but not useful to think of it that way.)
    //      the point is... "leap days up until that year". that allows us to accuratly determine the year accounting for leap days... 
    //  
    //      once you have the year, you can calculate number of seconds to start of year... and again, removing the leap days prior to the current year allows us to simply multiply number of years since epoch * seconds_per_non_leap_year.
    //      
    //      so then, we can calculate seconds_since_year_start by subtracting the seconds to start of year from the epoch seconds that has been adjusted (leap days subtracted....)
    //      
    //      so, what you have is if, leap year and before leap day... 
    //      the calendar is exactly the same as if non-leap year.
    //      if leap year and after leap day...
    //      the calendar has an extra day, but also will the seconds_since_year_start.
    //
    //      the calendar maps to a specific time since year start... and so what changes is that mapping... in that...
    //      seconds to year start == 3/1 is 2/29 instead..
    //      similarly, 1/1/<of_year_after_leap_year> is 1/1/<of_year_after_leap_adj_year> (12/31/<of_leap_year>) + 1 day.
    //      but again, we account for that by calculating the year properly, by first calculating number of leap years since epoch. so, if you were to try to get the time on 1/1/2025, it would see that there have been 14 leap years since epoch and remove the extra day from the calculation...

    uint32_t seconds_since_year_start = leap_adj_seconds - (year_since_1970 * SECONDS_PER_NON_LEAP_YEAR);
    uint32_t seconds_up_until_month;
    uint8_t month;
    uint8_t i = 0;

    const uint32_t * runningSecondsUpUntilMonth;
    if (false == isLeapYear(year)) {
        runningSecondsUpUntilMonth = &(runningSecondsUpUntilMonthNonLeapYear[0]);
    } else {
        runningSecondsUpUntilMonth = &(runningSecondsUpUntilMonthLeapYear[0]);
    }
    while (i < MONTHS_IN_YEAR && runningSecondsUpUntilMonth[i] < seconds_since_year_start) {
        i++;
    }
    seconds_up_until_month = runningSecondsUpUntilMonth[i - 1];
    month = i + 1;

     // seconds_since_year_start will always be less than seconds_up_until next month because of the above... so, I think we can safely add 1 to the day index.
    uint32_t seconds_since_month_start = seconds_since_year_start - seconds_up_until_month;
    uint8_t day = static_cast<uint8_t>(seconds_since_month_start/SECONDS_PER_DAY);

    uint32_t seconds_since_day_start = seconds_since_month_start - day * SECONDS_PER_DAY;
    uint8_t hr = static_cast<uint8_t>(seconds_since_day_start/SECONDS_PER_HOUR);
    uint8_t min = static_cast<uint8_t>((seconds_since_day_start - hr * SECONDS_PER_HOUR) / SECONDS_PER_MINUTE);
    uint8_t sec = static_cast<uint8_t>(seconds_since_day_start - hr * SECONDS_PER_HOUR - min * SECONDS_PER_MINUTE);

    // Month Day Year, 24H:60M:60S
    return WylesLibs::format("{s} {u} {u}, {u}:{u}:{u}", monthName[month], day + 1, year, hr, min, sec);
}