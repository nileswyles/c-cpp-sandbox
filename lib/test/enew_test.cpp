#include <stdint.h>
#include <stdio.h>
#include <new>

#include "logger.h"

#ifndef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_TEST
#endif

// TODO:
//  yeah, add more tests...

int main() {
    // uint16_t size = 65536 / 4;
    uint16_t size = DYNAMIC_MEMORY_SIZE;
    loggerPrintf(LOGGER_TEST, "size: %u\n", size);
    uint8_t * ptr[DYNAMIC_MEMORY_SIZE];
    uint8_t vals[DYNAMIC_MEMORY_SIZE];
    for (int i = 0; i < size+1; i++) {
        loggerPrintf(LOGGER_DEBUG, "*********************new iteration: %d\n", i);
        try {
            ptr[i] = new uint8_t(i);
            vals[i] = *(ptr[i]);
        } catch (const std::bad_alloc& e) {
            loggerPrintf(LOGGER_TEST, "exception explanation: %s\n", e.what());

        }
    }

    loggerPrintByteArray(LOGGER_TEST, vals, size);

    for (int i = 0; i < size-2; i++) {
        loggerPrintf(LOGGER_DEBUG, "*********************delete iteration: %d\n", i);
        delete ptr[i];
    }

    for (int i = 0; i < size; i++) {
        loggerPrintf(LOGGER_DEBUG, "*********************new2 iteration: %d\n", i);
        try {
            ptr[i] = new uint8_t((size * 2) - i);
            vals[i] = *(ptr[i]);
        } catch (const std::bad_alloc& e) {
            loggerPrintf(LOGGER_TEST, "exception explanation: %s\n", e.what());
        }
    }

    loggerPrintByteArray(LOGGER_TEST, vals, size);

    return 0;
}