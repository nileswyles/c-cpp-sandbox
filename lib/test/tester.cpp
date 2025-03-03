#include "tester.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>
#include <limits.h>

#include <iostream>

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED 1
#include "logger.h"

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

extern void WylesLibs::Test::ASSERT_UINT64(TestArg * t, uint64_t result, uint64_t expected) {
    loggerPrintf(LOGGER_TEST_VERBOSE, "Result:\n%lu\n", result);
    loggerPrintf(LOGGER_TEST_VERBOSE, "Expected:\n%lu\n", expected);

    if (result == expected) {
        t->fail = false;
    }
}

bool Tester::run(const char * name) {
    // per suite header section
    std::string heading = "\n";
    for (size_t i = 0; i < 20; i++) {
        heading += "-";
    }
    heading += " ";
    heading += this->suite_name;
    heading += " ";
    for (size_t i = 0; i < 20; i++) {
        heading += "-";
    }
    heading += "\n";
    printf("%s", heading.c_str());

    if (this->before != nullptr) {
        this->before();
    }
    size_t num_failed = 0;
    size_t num_passed = 0;

    // per suite tests section
    std::string failed_names;
    for (auto test: this->tests) {
        bool ran_test = false;
        // TODO: test selection pattern matching? list of tests?
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
            loggerPrintf(LOGGER_TEST, "Exception: %s\n", e.what());
            loggerExec(LOGGER_TEST, 
                printf("\n\nTest Failed!\n");
            );
        }
        if (ran_test) {
            if (test.arg.fail) {
                failed_names += '\t';
                failed_names += test.name;
                failed_names += " -> ";
                if (test.function_location.size() == 0) {
                    failed_names += WylesLibs::format("{s}:{d}", test.file_name.c_str(), test.line_number);
                } else {
                    failed_names += test.function_location;
                }
                failed_names += '\n';
                num_failed++;
            } else {
                num_passed++;
            }
        }
    }

    // per suite results section
    loggerExec(LOGGER_TEST,
        printf("\n#######################################\n");
    );
    // per suite results->failed tests section
    if (num_failed > 0) {
        printf("\n Failed Tests: \n\n");
        printf("%s", failed_names.c_str());
    }
    // per suite results->results section
    printf("\n Results: %lu passed, %lu failed\n", num_passed, num_failed);

    if (this->after != nullptr) {
        this->after();
    }

    // per suite footer section
    std::string footing = "\n";
    for (size_t i = 0; i < this->suite_name.size() + 2 + 40; i++) {
        footing += "-";
    }
    footing += "\n";
    printf("%s", footing.c_str());
    // printf("%s", heading.c_str());
    return num_failed == 0;
}

void Tester::runTest(Test * test) {
    // per test header section
    loggerExec(LOGGER_TEST,
        printf("\n#######################################\n\n");
        printf("Test Func: %s\n\n", test->name.c_str());
        printf("Initialization -> %s:%d\n", test->file_name.c_str(), test->line_number);
        if (test->declaration_location.size() > 0) {
            printf("Declaration -> %s\n", test->declaration_location.c_str());
        }
        if (test->function_location.size() > 0) {
            printf("Definition -> %s\n", test->function_location.c_str());
        }
        printf("\n");
    );
    if (this->before_each != nullptr) {
        this->before_each(&test->arg);
    }
    test->func(&test->arg);
    // per test results section
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