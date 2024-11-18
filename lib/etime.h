#ifndef WYLESLIBS_ETIME_H
#define WYLESLIBS_ETIME_H

#include <stdint.h>

namespace WylesLibs::Cal {
    extern void setApplicationTimeOffset(int8_t offset);
    extern uint64_t getUTCEpochTime();
    extern uint64_t getZonedEpochTime(int8_t offset);
};

#endif