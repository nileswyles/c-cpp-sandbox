#ifndef LOGGER_H
#define LOGGER_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

// #include <string.h>

#ifndef LOGGER_LEVEL
// default to "LOGGER_ERROR"
#define LOGGER_LEVEL 0
#endif

// TODO: portability? what other interesting things can we do with macros?
// NOTE: ## removes trailing comma when args is empty.

//  
// track list of files at top of translation unit...
//  each file that includes logger.h must add file to list using a macro...
//  
// #ifndef LOGGER_FILES_BUFFER_SIZE
// #define LOGGER_FILES_BUFFER_SIZE 1024
// #endif

// static char LOGGER_FILES[LOGGER_FILES_BUFFER_SIZE] = {0};
// static size_t LOGGER_FILES_CURSOR = 0;

// static void LOGGER_MODULE_ENABLE(const char * file) {
//     size_t file_name_size = strlen(file);
//     if (file_name_size + LOGGER_FILES_CURSOR + 1 < LOGGER_FILES_BUFFER_SIZE) {
//         strcpy(LOGGER_FILES + LOGGER_FILES_CURSOR, file); 
//         LOGGER_FILES_CURSOR += file_name_size;
//         LOGGER_FILES[LOGGER_FILES_CURSOR++] = ',';
//     }
// }
// #define LOGGER_MODULE_ENABLE(file) \
//     size_t file_name_size = strlen(file); \
//     if (file_name_size + LOGGER_FILES_CURSOR + 1 < LOGGER_FILES_BUFFER_SIZE) { \
//         strcpy(LOGGER_FILES + LOGGER_FILES_CURSOR, file); \
//         LOGGER_FILES_CURSOR += file_name_size; \
//         LOGGER_FILES[LOGGER_FILES_CURSOR++] = ','; \
//     }

// static bool IS_LOGGER_MODULE_ENABLED(const char * file) {
//     return strstr(LOGGER_FILES, file) != NULL;
// }

// Been meaning to document this... Not sure how __LINE__ evaluates to the correct line number, but it's working for now?
#define loggerPrintf(min, fmt, ...) \
    if (LOGGER_LEVEL >= min) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_TEST) {\
            file = stdout;\
        }\
        fprintf(file, "%s:%d (%s) " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);\
    }

#define loggerPrintByteArray(min, arr, size) \
    if (LOGGER_LEVEL >= min) {\
        FILE * file = stderr;\
        if (LOGGER_LEVEL >= LOGGER_TEST) {\
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
    LOGGER_TEST_VERBOSE,
    LOGGER_DEBUG,
    LOGGER_DEBUG_VERBOSE,
} log_level;

#endif