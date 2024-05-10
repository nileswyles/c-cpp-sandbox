#include <stdint.h>
#include <stdio.h>
#include <new>

#include "logger.h"

#ifndef LOGGER_LEVEL
#define LOGGER_LEVEL LOGGER_TEST
#endif

int main() {
    // uint16_t size = 65536 / 4;
    uint16_t size = 16;
    logger_printf(LOGGER_TEST, "size: %u\n", size);
    uint8_t * ptr[16];
    uint8_t vals[16];
    for (int i = 0; i < size+1; i++) {
        logger_printf(LOGGER_DEBUG, "*********************new iteration: %d\n", i);
        try {
            ptr[i] = new uint8_t(i);
            vals[i] = *(ptr[i]);
        } catch (const std::bad_alloc& e) {
            logger_printf(LOGGER_TEST, "exception explanation: %s\n", e.what());

        }
    }

    logger_print_byte_array(LOGGER_TEST, vals, size);

    for (int i = 0; i < size-2; i++) {
        logger_printf(LOGGER_DEBUG, "*********************delete iteration: %d\n", i);
        // TODO: "type"-new function seg faults when malloc returns NULL... 
        delete ptr[i];
    }
    for (int i = 0; i < size; i++) {
        logger_printf(LOGGER_DEBUG, "*********************new2 iteration: %d\n", i);
        try {
            ptr[i] = new uint8_t((size * 2) - i);
            vals[i] = *(ptr[i]);
        } catch (const std::bad_alloc& e) {
            logger_printf(LOGGER_TEST, "exception explanation: %s\n", e.what());
        }
    }

    logger_print_byte_array(LOGGER_TEST, vals, size);
    return 0;
}