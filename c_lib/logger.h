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
#define LOGGER_LEVEL 1
#endif

// TODO: portability? what other interesting things can we do with macros?
// NOTE: ## removes trailing comma when args is empty.
#define logger_printf(fmt, ...) \
    if (LOGGER_LEVEL >= ERROR) { \
        printf(fmt, ##__VA_ARGS__); \
    }

#define logger_debug_printf(fmt, ...) \
    if (LOGGER_LEVEL >= DEBUG) { \
        printf(fmt, ##__VA_ARGS__); \
    }

typedef enum log_level {
    ERROR, // 0
    DEBUG,
} log_level;

extern void logger_print_array(uint8_t * arr, size_t size);

extern void logger_debug_print_array(uint8_t * arr, size_t size);

#endif

#if defined __cplusplus
}
#endif
