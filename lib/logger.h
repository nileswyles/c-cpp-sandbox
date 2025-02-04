#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>
#include <pthread.h>

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
#ifdef WYLESLIBS_LOGGER_TIME_DISABLED
#undef loggerPrintf
#define loggerPrintf(min, fmt, ...) \
    if (LOGGER_LEVEL >= min && LOGGER_MODULE_ENABLED) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_TEST) {\
            file = stdout;\
        }\
        fprintf(file, "[%s] (thread: %lu) {%s:%d} (%s) -> " fmt,\
            LOG_LEVEL_STRINGS[min],\
            pthread_self(),\
            __FILE__,\
            __LINE__,\
            __func__,\
            ##__VA_ARGS__\
        );\
    }
#else
#undef loggerPrintf
#define loggerPrintf(min, fmt, ...) \
    if (LOGGER_LEVEL >= min && LOGGER_MODULE_ENABLED) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_TEST) {\
            file = stdout;\
        }\
        fprintf(file, "%s [%s] (thread: %lu) {%s:%d} (%s) -> " fmt,\
            WylesLibs::Cal::getFormattedDateTime(0).c_str(),\
            LOG_LEVEL_STRINGS[min],\
            pthread_self(),\
            __FILE__,\
            __LINE__,\
            __func__,\
            ##__VA_ARGS__\
        );\
    }
#endif

#undef loggerPrintByteArray
#define loggerPrintByteArray(min, arr, size) \
    if (LOGGER_LEVEL >= min && LOGGER_MODULE_ENABLED) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_TEST) {\
            file = stdout;\
        }\
        fprintf(file, "%s {%s:%d} (thread: %lu) [%s] (%s) -> ",\
            WylesLibs::Cal::getFormattedDateTime(0).c_str(),\
            __FILE__,\
            __LINE__,\
            pthread_self(),\
            LOG_LEVEL_STRINGS[min],\
            __func__\
        );\
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
    if (LOGGER_LEVEL >= min && LOGGER_MODULE_ENABLED) {\
        code\
    }
