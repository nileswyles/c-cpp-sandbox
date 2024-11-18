#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "ecal.h"

#ifndef LOGGER_LEVEL
// default to "LOGGER_INFO"
#define LOGGER_LEVEL 0
#endif

#ifndef LOGGER_MODULE_ENABLED
// default to "LOGGER_INFO"
#define LOGGER_MODULE_ENABLED 1
#endif

#ifndef LOGGER_H
#define LOGGER_H
typedef enum log_level {
    LOGGER_INFO, // 0
    LOGGER_TEST,
    LOGGER_TEST_VERBOSE,
    LOGGER_DEBUG,
    LOGGER_DEBUG_VERBOSE, // 4
} log_level;
static const char * LOG_LEVEL_STRINGS[5] = {
    "LOGGER_INFO",
    "LOGGER_TEST",
    "LOGGER_TEST_VERBOSE",
    "LOGGER_DEBUG",
    "LOGGER_DEBUG_VERBOSE"
};
#endif

// TODO: portability? what other interesting things can we do with macros?
// NOTE: ## removes trailing comma when args is empty.

// Been meaning to document this... Not sure how __LINE__ evaluates to the correct line number, but it's working for now?
#ifdef WYLESLIBS_LOGGER_OLD_ENABLED
#undef loggerPrintf
#define loggerPrintf(min, fmt, ...) \
    if (LOGGER_MODULE_ENABLED && LOGGER_LEVEL >= min) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_TEST) {\
            file = stdout;\
        }\
        fprintf(file, "%s:%d (%s) " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    }
#else
#undef loggerPrintf
#define loggerPrintf(min, fmt, ...) \
    if (LOGGER_MODULE_ENABLED && LOGGER_LEVEL >= min) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_TEST) {\
            file = stdout;\
        }\
        fprintf(file, "%s:%d (%s) " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
        fprintf(file, "%s - %s:%d [%s] (%s) " fmt,\
            WylesLibs::Cal::getFormattedDateTime(0).c_str(),\
            __FILE__, __LINE__,\
            LOG_LEVEL_STRINGS[min],\
            __func__, ##__VA_ARGS__);\
    }
#endif

#undef loggerPrintByteArray
#define loggerPrintByteArray(min, arr, size) \
    if (LOGGER_MODULE_ENABLED && LOGGER_LEVEL >= min) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_TEST) {\
            file = stdout;\
        }\
        fprintf(file, "%s - %s:%d [%s] (%s) ",\
            WylesLibs::Cal::getFormattedDateTime(0).c_str(),\
            __FILE__, __LINE__,\
            LOG_LEVEL_STRINGS[min],\
            __func__);\
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

#undef loggerExec
#define loggerExec(min, code) \
    if (LOGGER_MODULE_ENABLED && LOGGER_LEVEL >= min) {\
        code\
    }
