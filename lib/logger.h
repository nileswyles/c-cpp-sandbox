#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef LOGGER_LEVEL
// default to "LOGGER_ERROR"
#define LOGGER_LEVEL 0
#endif

// TODO: portability? what other interesting things can we do with macros?
// NOTE: ## removes trailing comma when args is empty.

// Been meaning to document this... Not sure how __LINE__ evaluates to the correct line number, but it's working for now?
#define loggerPrintf(min, fmt, ...) \
    if (LOGGER_LEVEL >= min) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_DEBUG) {\
            file = stdout;\
        }\
        fprintf(file, "%s:%d (%s) " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    }

#define loggerPrintByteArray(min, arr, size) \
    if (LOGGER_LEVEL >= min) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_DEBUG) {\
            file = stdout;\
        }\
        fprintf(file, "%s:%d (%s) ", __FILE__, __LINE__, __func__);\
        for (size_t i = 0; i < size; i++) {\
            char c = ((char *)arr)[i];\
            if (c <= 0x20 || c >= 0x7F) {\
                fprintf(file, "[%02X]", c & 0xFF);\
            } else {\
                fprintf(file, "%c", c);\
            }\
        }\
        fprintf(file, "\n");\
    }

typedef enum log_level {
    LOGGER_ERROR, // 0
    LOGGER_TEST,
    LOGGER_DEBUG,
} log_level;

#endif