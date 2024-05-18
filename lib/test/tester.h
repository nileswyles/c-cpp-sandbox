#ifndef WYLESLIBS_TESTER_H
#define WYLESLIBS_TESTER_H

#include "result.h"

#include <string>
#include <vector>
#include <stddef.h>
#include <stdbool.h>

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

class Tester {
    private:
        void runTest(Test * test);
    public:
        SuiteFunction * before;
        TestFunction * before_each;
        SuiteFunction * after;
        TestFunction * after_each;
        std::vector<Test> tests;
        size_t num_tests;

        Tester() {}
        Tester(SuiteFunction * before, TestFunction * before_each, SuiteFunction * after, TestFunction * after_each): 
            before(before), before_each(before_each), after(after), after_each(after_each), num_tests(0) {}

        void addTestWithName(const char * name, TestFunction * func) {
            std::string s(name);
            Test test = {.name = s, .func = func, .arg = { .fail = true }};
            this->tests.push_back(test);
            this->num_tests++;
        }
        void run(const char * name);
};

}

#endif