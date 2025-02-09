#ifndef WYLESLIBS_TESTER_H
#define WYLESLIBS_TESTER_H

#include "datastructures/array.h"
#include "cmder.h"
#include "logger.h"

#include <string>
#include <vector>
#include <stddef.h>
#include <stdbool.h>
#include <signal.h>

#define addTest(func)\
    addTestWithName(#func, func, __FILE__, __LINE__);

namespace WylesLibs::Test {

typedef struct TestArg {
    bool fail;
} TestArg;

typedef void (SuiteFunction)();
typedef void (TestFunction)(TestArg *);

typedef struct Test {
    std::string file_name;
    int line_number;
    std::string function_location;
    std::string declaration_location;
    std::string name;
    TestFunction * func;
    TestArg arg;
} Test;

extern void ASSERT_STRING(TestArg * t, std::string result, std::string expected);
extern void ASSERT_BOOLEAN(TestArg * t, bool result, bool expected);
extern void ASSERT_UINT64(TestArg * t, uint64_t result, uint64_t expected);

template<typename T>
extern void ASSERT_ARRAY(TestArg * t, SharedArray<T> result, SharedArray<T> expected) {
    if (expected.size() != result.size()) {
        t->fail = true;
        return;
    }
    for (size_t i = 0; i < expected.size(); i++) {
        loggerPrintf(LOGGER_TEST_VERBOSE, "Expected: 0x%X\n", expected[i]);
        loggerPrintf(LOGGER_TEST_VERBOSE, "Actual: 0x%X\n", result[i]);
        if (expected[i] != result[i]) {
            t->fail = true;
            return;
        }
    }
    t->fail = false;
}

// static void sig_handler(int sig, siginfo_t * info, void * context) {
//     // if (sig == SIGSEGV) {
//         loggerPrintf(LOGGER_DEBUG, "SIGSEGV, status: %d, int: %d, addr: %p\n", info->si_status, info->si_errno, info->si_addr);
//     // }
// }

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
                // act = {0};
                // act.sa_flags = SA_SIGINFO;
                // act.sa_sigaction = &sig_handler;
                // if (sigaction(SIGSEGV, &act, NULL) == -1) {
                //     loggerPrintf(LOGGER_DEBUG, "Failed to configure sig action and sig handler.\n");
                // }
        }
        Tester(std::string suite_name): Tester(suite_name, nullptr, nullptr, nullptr, nullptr) {}

        void addTestWithName(const char * name, TestFunction * func, const char * file_name, int line_number) {
            std::string test_name(name);
            std::string test_file_name(file_name);
            Test test = {
                .file_name = test_file_name, 
                .line_number = line_number,
                // TODO: hmm... would be nice to have a string containing cmd with args for logging
                //   so maybe need to process that anyways... so maybe keep the params as strings?
                //      or not... alternatively, implement toString specialization? (and concat) lol... 
                .function_location = esystem("/scripts/cpp-reflection/tester_function_mapper.pl", {
                    // ruh-roh
                    const_cast<char *>(("-p " + test_file_name).c_str()), 
                    const_cast<char *>(("-t " + test_name).c_str())
                }), 
                .declaration_location = esystem("/scripts/cpp-reflection/tester_declaration_mapper.pl", {
                    const_cast<char *>(("-p " + test_file_name).c_str()), 
                    const_cast<char *>(("-t " + test_name).c_str())
                }), 
                .name = test_name, 
                .func = func, 
                .arg = { .fail = TESTER_DEFAULT_TEST_FAIL_VALUE }
            };
            this->tests.push_back(test);
            this->num_tests++;
        }
        bool run(const char * name);
};

}

#endif