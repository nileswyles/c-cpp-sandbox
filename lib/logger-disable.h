#include "logger.h"

// lol... the compiler warning is useless but this is better than disabling the warning.
#undef loggerPrintf
#define loggerPrintf(min, fmt, ...)

#undef loggerPrintByteArray
#define loggerPrintByteArray(min, arr, size)