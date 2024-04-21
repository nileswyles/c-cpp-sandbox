#ifndef TESTER_H
#define TESTER_H

#if defined __cplusplus
extern "C"
{
#endif

#include "array.h"
#include <stddef.h>
#include <stdbool.h>

#ifdef TEST_DEBUG
#define LOGGER_LEVEL LOGGER_DEBUG
#endif

#include "logger.h"

typedef struct Test {
    const char * name;
    test_function * func;
    bool fail;
} Test;

typedef struct Tester {
    test_function * before;
    test_function * beforeEach;
    test_function * after;
    test_function * afterEach;
    Array * tests;
    size_t num_tests;
    size_t num_passed;
    size_t num_failed;
} Tester;

// test suite
// -> tests

typedef void test_function(Tester *);

#define tester_add_test(t, func)\
    tester_add_test_with_name(t, #func, func);

extern Tester * tester_constructor(test_function * before, test_function * beforeEach, test_function * after, test_function * afterEach);
extern void tester_destructor(Tester * t);
extern void tester_add_test_with_name(Tester *t, const char * name, test_function * func);
extern void tester_run(Tester * t);

#if defined __cplusplus
}
#endif

#endif