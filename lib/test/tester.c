#include "tester.h"
#include <stdlib.h>
#include <string.h>

static void run(Tester * tester, Test * test);

extern Tester * tester_constructor(suite_function * before, test_function * beforeEach, suite_function * after, test_function * afterEach) {
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

extern void tester_add_test_with_name(Tester * t, const char * name, test_function * func) {
    Test test = {.name = name, .func = func, .arg = { .fail = true }};
    if (OPERATION_SUCCESS == array_append(t->tests, (void *)&test, 1)) {
        // only increment if successful 
        t->num_tests++;
    }
}

// this should be more than enough... these are tests. just update if a test suite requires more...
#define FAILED_NAMES_BUFFER_SIZE 27000

extern void tester_run(Tester * t, const char * name) {
    if (t->before != NULL) {
        t->before();
    }
    size_t num_failed = 0;
    size_t num_passed = 0;

    static char failed_names[FAILED_NAMES_BUFFER_SIZE] = {};
    size_t failed_names_cursor = 0;

    for (size_t i = 0; i < t->num_tests; i++) {
        bool ran_test = false;
        Test * test = (Test *)(t->tests->buf) + i;
        if (name == NULL) {
            run(t, test);
            ran_test = true;
        } else if (strcmp(test->name, name) == 0) {
            run(t, test);
            ran_test = true;
        }
        if (ran_test) {
            if (test->arg.fail) {
                strcpy(failed_names + failed_names_cursor, test->name);
                failed_names_cursor += strlen(test->name) + 1;
                num_failed++;
            } else {
                num_passed++;
            }
        }
    }

    printf("\n#######################################\n");

    // TODO: CPP classes and strings... might complicate build scripts?
    if (num_failed > 0) {
        printf("\n Failed Tests: \n\n");
        for (size_t i = 0; i < failed_names_cursor; i++) {
            char c = failed_names[i];
            bool start_of_line = true;
            if (c == 0x00) {
                printf("\n");
                start_of_line = true;
            } else {
                if (start_of_line) {
                    printf("\t");
                    start_of_line = false;
                }
                printf("%c", failed_names[i]);
            }
        }
    }

    printf("\n Results: %lu passed, %lu failed\n", num_passed, num_failed);
    if (t->after != NULL) {
        t->after();
    }

    printf("\n---------------------------------------\n");
}

static void run(Tester * tester, Test * test) {
    printf("\n#######################################\n");
    printf("\nTest Func: %s\n\n", test->name);
    if (tester->beforeEach != NULL) {
        tester->beforeEach(&test->arg);
    }
    test->func(&test->arg);
    if (test->arg.fail) {
        printf("\nTest Failed!\n");
    } else {
        printf("\nTest Passed!\n");
    }
    if (tester->afterEach != NULL) {
        tester->afterEach(&test->arg);
    }
    // printf("\n#######################################\n");
}