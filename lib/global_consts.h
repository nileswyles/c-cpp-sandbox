#ifndef WYLESLIBS_GLOBAL_CONSTS_H
#define WYLESLIBS_GLOBAL_CONSTS_H

// this is still an arbitrary limit chosen based on number of digits of 2**32.
//  it also satisfies requirement of single precision significand size (2**24) which is well within 10 digits (10**10 - 1).
static constexpr size_t NUMBER_MAX_DIGITS = 10;

#endif