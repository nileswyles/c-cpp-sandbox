#ifndef TESTER_H
#define TESTER_H

#if defined __cplusplus
extern "C"
{
#endif

#include "c/array.h"
#include <stddef.h>
#include <stdbool.h>

#ifndef LOGGER_LEVEL 
#define LOGGER_LEVEL LOGGER_TEST
#endif

#include "logger.h"

typedef struct TestArg {
    bool fail;
} TestArg;

typedef void (suite_function)();
typedef void (test_function)(TestArg *);

typedef struct Test {
    const char * name;
    test_function * func;
    TestArg arg;
} Test;

typedef struct Tester {
    suite_function * before;
    test_function * beforeEach;
    suite_function * after;
    test_function * afterEach;
    Array * tests;
    size_t num_tests;
} Tester;

// test suite
// -> tests

#define tester_add_test(t, func)\
    tester_add_test_with_name(t, #func, func);

extern Tester * tester_constructor(suite_function * before, test_function * beforeEach, suite_function * after, test_function * afterEach);
extern void tester_destructor(Tester * t);
extern void tester_add_test_with_name(Tester *t, const char * name, test_function * func);

// TODO: will I ever want to support a list of test names via CLI argument?
extern void tester_run(Tester * t, const char * name);

#if defined __cplusplus
}
#endif

#endif