#ifndef WYLESLIBS_TESTER_H
#define WYLESLIBS_TESTER_H

#include <string>
#include <vector>
#include <stddef.h>
#include <stdbool.h>
#include <signal.h>
#include "logger.h"

#define addTest(func)\
    addTestWithName(#func, func, __FILE__, __LINE__);

namespace WylesLibs::Test {

typedef struct TestArg {
    bool fail;
} TestArg;

typedef void (SuiteFunction)();
typedef void (TestFunction)(TestArg *);

typedef struct Test {
    std::string test_file_name;
    int line_number;
    std::string name;
    TestFunction * func;
    TestArg arg;
} Test;

extern void ASSERT_STRING(TestArg * t, std::string result, std::string expected);
extern void ASSERT_BOOLEAN(TestArg * t, bool result, bool expected);

static void sig_handler(int sig, siginfo_t * info, void * context) {
    // if (sig == SIGSEGV) {
        loggerPrintf(LOGGER_DEBUG, "SIGSEGV, status: %d, int: %d, addr: %p\n", info->si_status, info->si_errno, info->si_addr);
    // }
}

#define TESTER_DEFAULT_TEST_FAIL_VALUE true
class Tester {
    private:
        struct sigaction act;
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
            suite_name(suite_name), before(before), before_each(before_each), after(after), after_each(after_each), num_tests(0) {
                act = {0};
                act.sa_flags = SA_SIGINFO;
                act.sa_sigaction = &sig_handler;
                if (sigaction(SIGSEGV, &act, NULL) == -1) {
                    loggerPrintf(LOGGER_DEBUG, "Failed to configure sig action and sig handler.\n");
                }
        }
        Tester(std::string suite_name): Tester(suite_name, nullptr, nullptr, nullptr, nullptr) {}

        void addTestWithName(const char * name, TestFunction * func, const char * test_file_name, int line_number) {
            std::string s(name);
            Test test = {.test_file_name = std::string(test_file_name), .line_number = line_number, .name = s, .func = func, .arg = { .fail = TESTER_DEFAULT_TEST_FAIL_VALUE }};
            this->tests.push_back(test);
            this->num_tests++;
        }
        bool run(const char * name);
};

}

#endif