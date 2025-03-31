#ifndef WYLESLIBS_ECAL_H
#define WYLESLIBS_ECAL_H

#include <stdint.h>
#include <string>

namespace WylesLibs::Cal {
    typedef enum DATETIME_FORMAT {
        READABLE, // Month Day Year, hh:mm:ss TIMEZONE
        ISO8601, // iso8601
        ISO8601_READABLE // iso8601 with separators
    } DATETIME_FORMAT; 

    extern std::string getFormattedDateTime(int16_t offset = 0, DATETIME_FORMAT format = READABLE);
    extern uint64_t getEpochFromFormattedDateTime(std::string dt, DATETIME_FORMAT format = READABLE);
};

#endif