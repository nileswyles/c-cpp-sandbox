#include "logger.h"

inline void __logger_print_array(uint8_t * arr, size_t size, const char * func) {
    FILE * file = stderr;
    if (LOGGER_LEVEL >= DEBUG) {
        file = stdout;
    }
    fprintf(file, "%s:%d (%s) ", __FILE__, __LINE__, func);
    for (size_t i = 0; i < size; i++) {
        char c = ((char *)arr)[i];
        if (c <= 0x20) {
            fprintf(file, "[%x]", c);
        } else {
            fprintf(file, "%c", c);
        }
    }
    fprintf(file, "\n");
}