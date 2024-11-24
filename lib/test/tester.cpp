#include "tester.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

using namespace WylesLibs::Test;

extern void WylesLibs::Test::ASSERT_STRING(TestArg * t, std::string result, std::string expected) {
    loggerPrintf(LOGGER_TEST_VERBOSE, "Result:\n'%s'\n", result.c_str());
    loggerPrintf(LOGGER_TEST_VERBOSE, "Expected:\n'%s'\n", expected.c_str());

    if (result == expected) {
        t->fail = false;
    }
}

extern void WylesLibs::Test::ASSERT_BOOLEAN(TestArg * t, bool result, bool expected) {
    loggerPrintf(LOGGER_TEST_VERBOSE, "Result:\n%s\n", result ? "true": "false");
    loggerPrintf(LOGGER_TEST_VERBOSE, "Expected:\n%s\n", expected ? "true" : "false");

    if (result == expected) {
        t->fail = false;
    }
}

bool Tester::run(const char * name) {
    printf("\n-------------------- %s --------------------\n", this->suite_name.c_str());
    if (this->before != nullptr) {
        this->before();
    }
    size_t num_failed = 0;
    size_t num_passed = 0;

    std::string failed_names;
    for (auto test: this->tests) {
        bool ran_test = false;
        // TODO: test selection pattern matching?
        try {
            if (name == nullptr) {
                runTest(&test);
                ran_test = true;
            } else if (test.name == std::string(name)) {
                runTest(&test);
                ran_test = true;
            }
        } catch(std::exception &e) {
            test.arg.fail = true;
            ran_test = true;
            loggerPrintf(LOGGER_INFO, "Exception: %s\n", e.what());
        }
        if (ran_test) {
            if (test.arg.fail) {
                failed_names += '\t';
                failed_names += test.name;
                failed_names += " -> ";
                failed_names += test.test_file_name;
                failed_names += ":";
                failed_names += std::to_string(test.line_number);
                failed_names += '\n';
                num_failed++;
            } else {
                num_passed++;
            }
        }
    }

    loggerExec(LOGGER_TEST,
        printf("\n#######################################\n");
    );
    if (num_failed > 0) {
        printf("\n Failed Tests: \n\n");
        printf("%s", failed_names.c_str());
    }
    printf("\n Results: %lu passed, %lu failed\n", num_passed, num_failed);

    if (this->after != nullptr) {
        this->after();
    }

    printf("\n---------------------------------------\n");
    return num_failed == 0;
}

void Tester::runTest(Test * test) {
    loggerExec(LOGGER_TEST,
        printf("\n#######################################\n");
        printf("\nTest Func: %s -> %s:%d\n\n", test->name.c_str(), test->test_file_name.c_str(), test->line_number);
    );
    if (this->before_each != nullptr) {
        this->before_each(&test->arg);
    }
    test->func(&test->arg);
    loggerExec(LOGGER_TEST,
        if (test->arg.fail) {
            printf("\n\nTest Failed!\n");
        } else {
            printf("\n\nTest Passed!\n");
        }
    );
    if (this->after_each != nullptr) {
        this->after_each(&test->arg);
    }
}