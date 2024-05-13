#include "tester.h"
#include <stdlib.h>

static void run(Tester * tester, Test * test);

extern Tester * tester_constructor(test_function * before, test_function * beforeEach, test_function * after, test_function * afterEach) {
    Tester * t = (Tester *)malloc(sizeof(Tester));
    t->before = before;
    t->beforeEach = beforeEach;
    t->after = after;
    t->afterEach = afterEach;
    t->num_tests = 0;
    t->tests = array_constructor(ARRAY_RECOMMENDED_INITIAL_CAP, sizeof(Test));
    return t;
}

extern void tester_destructor(Tester * t) {
    array_destructor(t->tests);
    free(t);
}

extern void tester_add_test_with_name(Tester *t, const char * name, test_function * func) {
    if (OPERATION_SUCCESS == array_append(t->tests, (void *)&(Test){.name = name, .func = func, .fail = true}, 1)) {
        // only increment if successful 
        t->num_tests++;
    }
}

extern void tester_run(Tester * t) {
    if (t->before != NULL) {
        t->before(t);
    }
    t->num_failed = 0;
    t->num_passed = 0;
    for (size_t i = 0; i < t->num_tests; i++) {
        run(t, (Test *)(t->tests->buf) + i);
    }
    printf("\n Results: %lu passed, %lu failed\n", t->num_passed, t->num_failed);
    if (t->after != NULL) {
        t->after(t);
    }
}

static void run(Tester * tester, Test * test) {
    printf("\n#######################################\n");
    printf("\nTest Func: %s\n", test->name);
    if (tester->beforeEach != NULL) {
        tester->beforeEach(tester);
    }
    test->func((void *)tester);
    if (test->fail) {
        printf("\nTest Failed!\n");
        tester->num_failed++;
    } else {
        printf("\nTest Passed!\n");
        tester->num_passed++;
    }
    if (tester->afterEach != NULL) {
        tester->afterEach(tester);
    }
    printf("\n#######################################\n");
}