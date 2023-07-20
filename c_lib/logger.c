#include "logger.h"

inline void logger_print_array(uint8_t * arr, size_t size) {
    if (LOGGER_LEVEL >= ERROR) {
        for (size_t i = 0; i < size; i++) {
            char c = ((char *)arr)[i];
            if (c <= 0x20) {
                printf("[%x]", c);
            } else {
                printf("%c", c);
            }
        }
    }
}

inline void logger_debug_print_array(uint8_t * arr, size_t size) {
    if (LOGGER_LEVEL >= DEBUG) {
        logger_print_array(arr, size);
    }
}
