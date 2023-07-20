#if defined __cplusplus
extern "C"
{
#endif

#ifndef LOGGER_H
#define LOGGER_H
#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#ifndef LOGGER_LEVEL
// default to "ERROR"
#define LOGGER_LEVEL 0
#endif

// TODO: portability? what other interesting things can we do with macros?
// NOTE: ## removes trailing comma when args is empty.
// TODO: __FILE__, __LINE__, __func__
#define logger_printf(fmt, ...) \
    fprintf(stderr, "%s:%d (%s) " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__);

#define logger_debug_printf(fmt, ...) \
    if (LOGGER_LEVEL >= DEBUG) { \
        fprintf(stdout, "%s:%d (%s) " fmt, __FILE__, __LINE__, __func__, ##__VA_ARGS__); \
    }

#define logger_print_array(arr, size) __logger_print_array(arr, size, __func__)

#define logger_debug_print_array(arr, size) logger_print_array(arr, size)

typedef enum log_level {
    ERROR, // 0
    DEBUG,
} log_level;

extern void __logger_print_array(uint8_t * arr, size_t size, const char * func);
// TODO: should I just keep this as a macro, to not need to include logger.c? 
    // FILE * file = stderr;\
    // if (LOGGER_LEVEL >= DEBUG) {\
    //     file = stdout;\
    // }\
    // fprintf(file, "%s:%d (%s) ", __FILE__, __LINE__, __func__);\
    // for (size_t i = 0; i < size; i++) {\
    //     char c = ((char *)arr)[i];\
    //     if (c <= 0x20) {\
    //         fprintf(file, "[%x]", c);\
    //     } else {\
    //         fprintf(file, "%c", c);\
    //     }\
    // }\
    // fprintf(file, "\n");

#endif

#if defined __cplusplus
}
#endif
