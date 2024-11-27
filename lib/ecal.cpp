#include "ecal.h"
#include "string_format.h"
#include "etime.h"
#include "estream/byteestream.h"

#include <string>

#include <stdint.h>

#define SECONDS_PER_HOUR 3600 // 60 * 60
#define SECONDS_PER_MINUTE 60
#define SECONDS_PER_DAY 86400 // 60 * 60 * 24
#define SECONDS_PER_NON_LEAP_YEAR 31536000 // 365 * 60 * 60 * 24

#define MONTHS_IN_YEAR 12
#define SIZE_MONTHS_ARRAY MONTHS_IN_YEAR + 1

using namespace WylesLibs;

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

static constexpr const char * MONTH_NAME[SIZE_MONTHS_ARRAY] = {
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

static const std::map<std::string, uint8_t> MONTH_NUM{
    {"Jan", 1},
    {"Feb", 2},
    {"Mar", 3},
    {"Apr", 4},
    {"May", 5},
    {"June" 6},
    {"July" 7},
    {"Aug", 8},
    {"Sep", 9},
    {"Oct", 10},
    {"Nov", 11},
    {"Dec", 12}
};

static bool isLeapYear(uint16_t year) {
    return year % 4 == 0 && (year % 100 != 0 || (year % 100 == 0 && year % 400 == 0));
}

static uint32_t secondsUpUntilMonth(uint8_t month, uint16_t year) {
    const uint32_t * running_seconds_up_until_month;
    if (true == isLeapYear(year)) {
        running_seconds_up_until_month = &(RUNNING_SECONDS_UP_UNTIL_MONTH_LEAP_YEAR[0]);
    } else {
        running_seconds_up_until_month = &(RUNNING_SECONDS_UP_UNTIL_MONTH_NON_LEAP_YEAR[0]);
    }
    return running_seconds_up_until_month[month] - 1;
}

extern std::string WylesLibs::Cal::getFormattedDateTime(int16_t offset, WylesLibs::Cal::DATETIME_FORMAT format) {
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

    std::string formatted_offset("Z");
    // ! IMPORTANT - offset value is validated and updated by the function used to get the time.
    if (format == WylesLibs::Cal::READABLE) {
        if (offset != 0) {
            formatted_offset = WylesLibs::format("{+d}:{-02d}", static_cast<int64_t>(offset/100), static_cast<int64_t>(offset));
        }
        return WylesLibs::format("{s} {u} {u}, {u}:{u}:{u} ", MONTH_NAME[month], static_cast<uint64_t>(day + 1), static_cast<uint64_t>(year), static_cast<uint64_t>(hr), static_cast<uint64_t>(min), static_cast<uint64_t>(sec)) + formatted_offset;
    } else if (format == WylesLibs::Cal::ISO8601_READABLE) {
        if (offset != 0) {
            formatted_offset = WylesLibs::format("{+d}:{-02d}", static_cast<int64_t>(offset/100), static_cast<int64_t>(offset));
        }
        return WylesLibs::format("{04u}-{02u}-{02u}T{02u}:{02u}:{02u}", static_cast<uint64_t>(year), static_cast<uint64_t>(month), static_cast<uint64_t>(day + 1), static_cast<uint64_t>(hr), static_cast<uint64_t>(min), static_cast<uint64_t>(sec)) + formatted_offset;
    } else {
        if (offset != 0) {
            formatted_offset = WylesLibs::format("{+d}", static_cast<int64_t>(offset));
        }
        return WylesLibs::format("{04u}{02u}{02u}T{02u}{02u}{02u}", static_cast<uint64_t>(year), static_cast<uint64_t>(month), static_cast<uint64_t>(day + 1), static_cast<uint64_t>(hr), static_cast<uint64_t>(min), static_cast<uint64_t>(sec)) + formatted_offset;
    }
}

static void isValidMonth(uint8_t month) {
    if (month > 12) {
        std::string msg = WylesLibs::format("Error parsing Readable Date Time, invalid month. {u}", month);
        loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}
static void isValidDay(uint8_t day) {
    if (day > 31) {
        std::string msg = WylesLibs::format("Error parsing Readable Date Time, invalid day. {u}", day);
        loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}
static void isValidHr(uint8_t hr) {
    if (hr > 23) {
        std::string msg = WylesLibs::format("Error parsing Readable Date Time, invalid hour. {u}", hr);
        loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}
static void isValidMin(uint8_t min) {
    if (min > 59) {
        std::string msg = WylesLibs::format("Error parsing Readable Date Time, invalid hour. {u}", min);
        loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}
static void isValidSec(uint8_t sec) {
    if (sec > 59) {
        std::string msg = WylesLibs::format("Error parsing Readable Date Time, invalid sec. {u}", sec);
        loggerPrintf(LOGGER_DEBUG, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }
}

static int16_t parseReadableOffset(ByteEStream &s) {
    int16_t offset = 0;
    // offset woop woop woop
    if (s.get() != "Z") {
        uint8_t sign = s.get();
        int8_t sign_mul = 0;
        if (sign == '+') {
            sign_mul = 1;
        } else if (sign == '-') {
            sign_mul = -1;
        } else {
            std::string msg = "Error parsing Readable Date Time, invalid sign.";
            loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
        int8_t hour_offset = static_cast<uint8_t>(s.readNatural());
        if (s.get() != ":") {
            std::string msg = "Error parsing Readable Date Time, expected ':'.";
            loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }// read space
        int8_t min_offset = static_cast<uint8_t>(s.readNatural());
        offset = sign_mul * (hour_offset * SECONDS_PER_HOUR + min_offset * SECONDS_PER_MIN);
    }
    return offset;
}

static uint64_t parseReadableDateTime(ByteEStream& s) {
    // {s} {u} {u}, {u}:{u}:{u}
    uint64_t epoch_seconds = 0;
    uint8_t month = MONTH_NUM[s.readString(" ")];
    isValidMonth(month);
    uint8_t day = static_cast<uint8_t>(s.readNatural());
    isValidDay(day);
    if (s.get() != " ") {
        std::string msg = "Error parsing Readable Date Time, expected space.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    } // read space

    uint16_t year = static_cast<uint16_t>(s.readNatural());
    isValidYear(year);
    if (s.get() != ",") {
        std::string msg = "Error parsing Readable Date Time, expected space.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }// read comma
    if (s.get() != " ") {
        std::string msg = "Error parsing Readable Date Time, expected space.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }// read space
    // adjust for year
    epoch_seconds += year * SECONDS_PER_NON_LEAP_YEAR;
    // adjust for month (accounting for leap year)
    epoch_seconds += secondsUpUntilMonth(month, year);
    epoch_seconds += day * SECONDS_PER_DAY;

    uint8_t hr = static_cast<uint16_t>(s.readNatural());
    isValidHr(hr);
    if (s.get() != " ") {
        std::string msg = "Error parsing Readable Date Time, expected space.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }// read space
    epoch_seconds += hr * SECONDS_PER_HOUR;

    uint8_t min = static_cast<uint16_t>(s.readNatural());
    isValidMin(min);
    if (s.get() != " ") {
        std::string msg = "Error parsing Readable Date Time, expected space.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }// read space
    epoch_seconds += min * SECONDS_PER_MIN;

    uint8_t sec = static_cast<uint16_t>(s.readNatural());
    isValidSec(sec);
    if (s.get() != " ") {
        std::string msg = "Error parsing Readable Date Time, expected space.";
        loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
        throw std::runtime_error(msg);
    }// read space
    epoch_seconds += sec;

    epoch_seconds += parseReadableOffset(s);

    return epoch_seconds;
}

static uint64_t parseISO8601ReadableDateTime(ByteEStream& s) {
    // {04u}-{02u}-{02u}T{02u}:{02u}:{02u}
    uint64_t epoch_seconds = 0;

    // TODO:
    //    hmm... on second thought, might not be good idea to use tuple here lol... get/tie/forward? wth is that?
    //    hmm...
    uint16_t year = s.readNatural();
    isValidYear(month);
    epoch_seconds += year * SECONDS_PER_NON_LEAP_YEAR;

    uint8_t month = s.readNatural("-");
    isValidMonth(month);
    epoch_seconds += secondsUpUntilMonth(month, year);

    uint8_t day = s.readNatural("T");
    isValidDay(day);
    epoch_seconds += day * SECONDS_PER_DAY;

    uint8_t hr = s.readNatural(":"));
    isValidHr(hr);
    epoch_seconds += hr * SECONDS_PER_HOUR;

    uint8_t min = s.readNatural(":");
    isValidMin(min);
    epoch_seconds += min * SECONDS_PER_MIN;

    uint8_t sec = s.readNatural(" ");
    isValidSec(sec);
    epoch_seconds += sec;

    epoch_seconds += parseReadableOffset(s);

    return epoch_seconds;
}

static uint64_t parseISO8601DateTime(ByteEStream& s) {
    // {04u}{02u}{02u}T{02u}{02u}{02u}
    uint64_t epoch_seconds = 0;

    uint16_t year = static_cast<uint16_t>(s.readNatural(4));
    isValidYear(month);
    epoch_seconds += year * SECONDS_PER_NON_LEAP_YEAR;

    uint8_t month = static_cast<uint16_t>(s.readNatural(2));
    isValidMonth(month);
    epoch_seconds += secondsUpUntilMonth(month, year);

    uint8_t day = static_cast<uint16_t>(s.readNatural(2));
    isValidDay(day);
    epoch_seconds += day * SECONDS_PER_DAY;

    uint8_t hr = static_cast<uint16_t>(s.readNatural(2));
    isValidHr(hr);
    epoch_seconds += hr * SECONDS_PER_HOUR;

    uint8_t min = static_cast<uint16_t>(s.readNatural(2));
    isValidMin(min);
    epoch_seconds += min * SECONDS_PER_MIN;

    uint8_t sec = static_cast<uint16_t>(s.readNatural(2));
    isValidSec(sec);
    epoch_seconds += sec;

    s.get();

    int16_t offset = 0;
    if (s.get() != "Z") {
        uint8_t sign = s.get();
        int8_t sign_mul = 0;
        if (sign == '+') {
            sign_mul = 1;
        } else if (sign == '-') {
            sign_mul = -1;
        } else {
            std::string msg = "Error parsing Readable Date Time, invalid sign.";
            loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
            throw std::runtime_error(msg);
        }
        // int8_t hour_offset = static_cast<uint8_t>(s.readNatural(1));
        // int8_t min_offset = static_cast<uint8_t>(s.readNatural(2));
        // epoch_seconds += sign_mul * (hour_offset * SECONDS_PER_HOUR + min_offset * SECONDS_PER_MIN);

        // order of operations? lol, I think this is valid... ( ͡° ͜ʖ ͡°) 
        epoch_seconds += sign_mul * (static_cast<uint8_t>(s.readNatural(1)) * SECONDS_PER_HOUR + static_cast<uint8_t>(s.readNatural(2)) * SECONDS_PER_MIN);
    }

    return epoch_seconds;
}

extern uint64_t getEpochFromFormattedDateTime(std::string dt, WylesLibs::Cal::DATETIME_FORMAT format = WylesLibs::Cal::READABLE) {
    ByteEStream s((uint8_t *)dt.data(), dt.size());
    if (format == WylesLibs::Cal::READABLE) {
        return parseReadableDateTime(s);
    } else if (format == WylesLibs::Cal::ISO8601_READABLE) {
        return parseISO8601ReadableDateTime(s);
    } else {
        return parseISO8601DateTime(s);
    }
}