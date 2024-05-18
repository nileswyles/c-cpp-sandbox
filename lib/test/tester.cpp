#include "tester.h"
#include <stdlib.h>
#include <string.h>
#include <stdio.h>

using namespace WylesLibs::Test;

void Tester::run(const char * name) {
    if (this->before != nullptr) {
        this->before();
    }
    size_t num_failed = 0;
    size_t num_passed = 0;

    std::string failed_names;
    for (auto test: this->tests) {
        bool ran_test = false;
        if (name == nullptr) {
            runTest(&test);
            ran_test = true;
        } else if (test.name == std::string(name)) {
            runTest(&test);
            ran_test = true;
        }
        if (ran_test) {
            if (test.arg.fail) {
                failed_names += '\t';
                failed_names += test.name;
                failed_names += '\n';
                num_failed++;
            } else {
                num_passed++;
            }
        }
    }
    printf("\n#######################################\n");

    if (num_failed > 0) {
        printf("\n Failed Tests: \n\n");
        printf("%s", failed_names.c_str());
    }

    printf("\n Results: %lu passed, %lu failed\n", num_passed, num_failed);
    if (this->after != nullptr) {
        this->after();
    }

    printf("\n---------------------------------------\n");
}

void Tester::runTest(Test * test) {
    printf("\n#######################################\n");
    printf("\nTest Func: %s\n\n", test->name.c_str());
    if (this->before_each != nullptr) {
        this->before_each(&test->arg);
    }
    test->func(&test->arg);
    if (test->arg.fail) {
        printf("\n\nTest Failed!\n");
    } else {
        printf("\n\nTest Passed!\n");
    }
    if (this->after_each != nullptr) {
        this->after_each(&test->arg);
    }
}