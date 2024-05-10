#include <stdint.h>
#include <stdio.h>
#include <new>

#include "logger.h"

#ifndef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_TEST
#endif

int main() {
    // uint16_t size = 65536 / 4;
    uint16_t size = 16 / 4;
    logger_printf(LOGGER_TEST, "size: %u\n", size);
    int * ptr[5];
    for (int i = 0; i < size+1; i++) {
        logger_printf(LOGGER_TEST, "\n*********************new iteration: %d\n", i);
        try {
            ptr[i] = new int(1);
        } catch (const std::bad_alloc& e) {
            logger_printf(LOGGER_TEST, "exception explanation: %s\n", e.what());

        }
    }
    for (int i = 0; i < size-2; i++) {
        logger_printf(LOGGER_TEST, "\n*********************delete iteration: %d\n", i);
        // TODO: "type"-new function seg faults when malloc returns NULL... 
        delete ptr[i];
    }
    for (int i = 0; i < size; i++) {
        logger_printf(LOGGER_TEST, "\n*********************new2 iteration: %d\n", i);
        try {
            ptr[i] = new int(1);
        } catch (const std::bad_alloc& e) {
            logger_printf(LOGGER_TEST, "exception explanation: %s\n", e.what());
        }
    }
    return 0;
}