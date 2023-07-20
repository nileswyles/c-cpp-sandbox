#ifndef TESTER_H
#define TESTER_H

#include "array.h"
#include <stddef.h>

typedef struct Test {
    const char * name;
    test_function * func;
} Test;

typedef struct Tester {
    test_function * before;
    test_function * beforeEach;
    test_function * after;
    test_function * afterEach;
    Array * tests;
    size_t num_tests;
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

#endif