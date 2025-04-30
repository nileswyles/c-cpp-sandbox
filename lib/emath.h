#ifndef WYLESLIBS_EMATH_H
#define WYLESLIBS_EMATH_H

#include <stdint.h>

namespace WylesLibs {
    int64_t normalize(const int64_t min, const int64_t max, const int64_t value, const int64_t new_min, const int64_t new_max) {
        double norm_per = (double)(value - min)/(double)(max + min);
        return (norm_per * (new_max - new_min)) + new_min;
    }
}
#endif