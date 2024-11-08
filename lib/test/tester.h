#ifndef WYLESLIBS_TESTER_H
#define WYLESLIBS_TESTER_H

#include <string>
#include <vector>
#include <stddef.h>
#include <stdbool.h>
#include "logger.h"

#define addTest(func)\
    addTestWithName(#func, func);

namespace WylesLibs::Test {

typedef struct TestArg {
    bool fail;
} TestArg;

typedef void (SuiteFunction)();
typedef void (TestFunction)(TestArg *);

typedef struct Test {
    std::string name;
    TestFunction * func;
    TestArg arg;
} Test;

extern void ASSERT_STRING(TestArg * t, std::string result, std::string expected);
extern void ASSERT_BOOLEAN(TestArg * t, bool result, bool expected);

#define TESTER_DEFAULT_TEST_FAIL_VALUE true
class Tester {
    private:
        std::string suite_name;
        void runTest(Test * test);
    public:
        SuiteFunction * before;
        TestFunction * before_each;
        SuiteFunction * after;
        TestFunction * after_each;
        std::vector<Test> tests;
        size_t num_tests;

        Tester(std::string suite_name, SuiteFunction * before, TestFunction * before_each, SuiteFunction * after, TestFunction * after_each): 
            suite_name(suite_name), before(before), before_each(before_each), after(after), after_each(after_each), num_tests(0) {}
        Tester(std::string suite_name): Tester(suite_name, nullptr, nullptr, nullptr, nullptr) {}

        void addTestWithName(const char * name, TestFunction * func) {
            std::string s(name);
            Test test = {.name = s, .func = func, .arg = { .fail = TESTER_DEFAULT_TEST_FAIL_VALUE }};
            this->tests.push_back(test);
            this->num_tests++;
        }
        bool run(const char * name);
};

}

#endif