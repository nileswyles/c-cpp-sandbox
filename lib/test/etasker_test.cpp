#include "tester.h"
#include "threads/etasker.h"

#include <set>

#include <signal.h>
#include <sys/sysinfo.h>
#include <time.h>

#ifndef LOGGER_ETASKER_TEST
#define LOGGER_ETASKER_TEST 1
#endif

#undef LOGGER_MODULE_ENABLED
#define LOGGER_MODULE_ENABLED LOGGER_ETASKER_TEST
#include "logger.h"

using namespace WylesLibs;
using namespace WylesLibs::Test;

static size_t num_runs = 0;
static size_t num_exited = 0;
static std::set<pthread_t> threadsSeen;
static std::set<pthread_t> threadsSeen2;

static pthread_mutex_t mutex = PTHREAD_MUTEX_INITIALIZER;

class TestETask: public ETask {
    public:
        TestETask() {}
        ~TestETask() {}
        void initialize() {
            sleep(5);
            loggerPrintf(LOGGER_DEBUG, "initialize called\n");
        }
        void run() {
            sleep(5);
            pthread_mutex_lock(&mutex);
            num_runs++;
            loggerPrintf(LOGGER_DEBUG, "run called, %lu\n", num_runs);
            pthread_mutex_unlock(&mutex);
        }
        void onExit() {
            sleep(5);
            pthread_mutex_lock(&mutex);
            num_exited++;
            loggerPrintf(LOGGER_DEBUG, "onExit called, %lu\n", num_exited);
            pthread_mutex_unlock(&mutex);
        }
};

void sig_handler1(int sig) {
    pthread_t pthread = pthread_self();
    if (ETasker::thread_specific_sig_handlers.contains(pthread)) {
        ETasker * etasker = ETasker::thread_specific_sig_handlers[pthread];
        // etasker->threadSigAction(sig, info, context);
        etasker->threadSigAction(sig, nullptr, nullptr);
    } else {
        raise(sig);
    }
}

uint64_t runTasksBursty(size_t thread_limit, uint64_t expected_elapsed_time, uint64_t expected_num_runs, bool fixed = false) {
    ETasker lol(thread_limit, expected_elapsed_time + 5, fixed);

    struct timespec prior;
    clock_gettime(CLOCK_MONOTONIC, &prior);

    ESharedPtr<TestETask> ptr(new TestETask());
    for (size_t i = 0; i < expected_num_runs/2; i++) {
        lol.run(ptr);
    }

    while (expected_num_runs/2 != num_exited) {
        for (auto i: ETasker::thread_specific_sig_handlers) {
            threadsSeen.insert(i.first);
        }
        usleep(270);
    }

    while (lol.threads() > 0) {}
    usleep(750); // needs to be enough to ensure threads are exited
    
    for (size_t i = expected_num_runs/2; i < expected_num_runs; i++) {
        lol.run(ptr);
    }

    while (expected_num_runs != num_exited) {
        for (auto i: ETasker::thread_specific_sig_handlers) {
            threadsSeen2.insert(i.first);
        }
        usleep(270);
    }

    while (lol.threads() > 0) {}
    usleep(270);

    struct timespec after;
    clock_gettime(CLOCK_MONOTONIC, &after);

    uint64_t elapsed_time_s = static_cast<uint64_t>(after.tv_sec - prior.tv_sec);

    loggerPrintf(LOGGER_TEST_VERBOSE, "Precise elapsed time: %lus, %luns\n", elapsed_time_s, after.tv_nsec - prior.tv_nsec);

    return elapsed_time_s;
}

uint64_t runTasks(size_t thread_limit, uint64_t expected_elapsed_time, uint64_t expected_num_runs, bool fixed = false) {
    ETasker lol(thread_limit, expected_elapsed_time + 5, fixed);

    struct timespec prior;
    clock_gettime(CLOCK_MONOTONIC, &prior);

    ESharedPtr<TestETask> ptr(new TestETask());
    for (size_t i = 0; i < expected_num_runs; i++) {
        lol.run(ptr);
    }

    while (expected_num_runs != num_exited) {
        for (auto i: ETasker::thread_specific_sig_handlers) {
            threadsSeen.insert(i.first);
        }
        usleep(270);
    }

    while (lol.threads() > 0) {}
    usleep(270);

    struct timespec after;
    clock_gettime(CLOCK_MONOTONIC, &after);

    uint64_t elapsed_time_s = static_cast<uint64_t>(after.tv_sec - prior.tv_sec);

    loggerPrintf(LOGGER_TEST_VERBOSE, "Precise elapsed time: %lus, %luns\n", elapsed_time_s, after.tv_nsec - prior.tv_nsec);

    return elapsed_time_s;
}

void assert(TestArg * t, uint64_t expected_elapsed_time, uint64_t actual_elapsed_time,  
                size_t expected_individual_threads, size_t actual_individual_threads, size_t expected_num_runs) {
    loggerPrintf(LOGGER_TEST, "Test Assertion: \n");
    loggerPrintf(LOGGER_TEST, "Actual Elapsed Time: %lu, Expected Elapsed Time: <=%lu\n", actual_elapsed_time, expected_elapsed_time);
    loggerPrintf(LOGGER_TEST, "Actual Num Runs: %lu, Expected Num Runs: %lu\n", num_runs, expected_num_runs);
    loggerPrintf(LOGGER_TEST, "Actual Individual Threads: %lu, Expected Individual Threads: %lu\n", actual_individual_threads, expected_individual_threads);

    if (actual_elapsed_time > expected_elapsed_time 
        || num_runs != expected_num_runs 
        || actual_individual_threads != expected_individual_threads) {
        t->fail = true;
    } else {
        t->fail = false;
    }
}

void testETasker(TestArg * t) {
    // hmm.. might limit to 1 to avoid flakiness or is there some queue limit at os that can be leveraged to ensure these are all "parallel"? 
    //      to ensure context switching happens at sleep function call.
    //      in a single threaded/single core machine
    //      thread 1 - - - - sleep
    //       2 - - - - - - - - sleep
    //       3 - - - - - - - - - sleep
    //       there's still some parallelism even though it's a single core. I'm pretty sure that's how it works... context switching only capable at os 

    //      otherwise, latency is a big problem when interacting with database.

    //      you might want the other configurations however for other server tasks.

    //      lol, again, let's revisit other models?
    //          long-running operations == socket tasks?
    //          so in http context -> geet fd from lissten, add fd to list of things to poll when information available, call parse request -> 
    //          then request passed to controller which does some other things
    //          let's say it accesses another server (another long-running operation)
    //          then you will add to queue which adds to poll list
    //          
    //          alright, following that train of thought, you might come up with something where you group into a list of events? 
    //              some threads polling and when fd == POLLIN you process some event via thread pool.
    //              so, then you need to worry about sharing state...
    //              in this example, the unit of work/event is something similar to how the functions in http are split up.

    //          another way to think about it is you have threads polling fds for async stuff... then you cond wait from other threads?

    //          hmm... sure that might work but not the correct place to context switch imo?

    //          right? proceeding agaain per usual...
    size_t procs_mul = 4;
    size_t procs = get_nprocs();

    size_t expected_individual_threads = procs * procs_mul;
    uint64_t expected_elapsed_time = 16;
    size_t expected_num_runs = procs * procs_mul;
    
    uint64_t actual_elapsed_time = runTasks(SIZE_MAX, expected_elapsed_time, expected_num_runs, false);

    size_t actual_individual_threads = threadsSeen.size();
    assert(t, expected_elapsed_time, actual_elapsed_time, expected_individual_threads, actual_individual_threads, expected_num_runs);
}

void testETaskerBursty(TestArg * t) {
    size_t procs = get_nprocs();
    size_t procs_mul = 2;

    size_t expected_individual_threads = procs * procs_mul; // in practice, it's not immediately re-using the pthread id's so keep like this but monitor
    uint64_t expected_elapsed_time = 16 * procs_mul;
    size_t expected_num_runs = procs * procs_mul;

    uint64_t actual_elapsed_time = runTasksBursty(SIZE_MAX, expected_elapsed_time, expected_num_runs, false);
    size_t actual_individual_threads = threadsSeen.size() + threadsSeen2.size();
    assert(t, expected_elapsed_time, actual_elapsed_time, expected_individual_threads, actual_individual_threads, expected_num_runs);
}

void testETaskerThreadLimit(TestArg * t) {
    size_t procs = get_nprocs();

    size_t expected_individual_threads = procs;
    uint64_t expected_elapsed_time = 16;
    size_t expected_num_runs = procs;

    uint64_t actual_elapsed_time  = runTasks(procs, expected_elapsed_time, expected_num_runs, false);
    size_t actual_individual_threads = threadsSeen.size();
    assert(t, expected_elapsed_time, actual_elapsed_time, expected_individual_threads, actual_individual_threads, expected_num_runs);
}

void testETaskerThreadLimitBursty(TestArg * t) {
    size_t procs = get_nprocs();
    size_t procs_mul = 2;

    size_t expected_individual_threads = procs * procs_mul; // in practice, it's not immediately re-using the pthread id's so keep like this but monitor
    uint64_t expected_elapsed_time = 16 * procs_mul;
    size_t expected_num_runs = procs * 2;

    uint64_t actual_elapsed_time = runTasksBursty(procs, expected_elapsed_time, expected_num_runs, false);

    size_t actual_individual_threads = threadsSeen.size() + threadsSeen2.size();
    assert(t, expected_elapsed_time, actual_elapsed_time, expected_individual_threads, actual_individual_threads, expected_num_runs);
}

void testETaskerThreadLimitPastLimit(TestArg * t) {
    size_t procs = get_nprocs();
    size_t procs_mul = 4;

    size_t expected_individual_threads = procs;
    uint64_t expected_elapsed_time = 16 * procs_mul;
    size_t expected_num_runs = procs * procs_mul;

    uint64_t actual_elapsed_time  = runTasks(procs, expected_elapsed_time, expected_num_runs, false);
    size_t actual_individual_threads = threadsSeen.size();
    assert(t, expected_elapsed_time, actual_elapsed_time, expected_individual_threads, actual_individual_threads, expected_num_runs);
}

void testETaskerThreadLimitFixedBursty(TestArg * t) {
    size_t procs = get_nprocs();
    size_t procs_mul = 2;

    size_t expected_individual_threads = procs * procs_mul;
    uint64_t expected_elapsed_time = 16 * procs_mul;
    size_t expected_num_runs = procs * 2;

    uint64_t actual_elapsed_time = runTasksBursty(procs, expected_elapsed_time, expected_num_runs, true);

    size_t actual_individual_threads = threadsSeen.size() + threadsSeen2.size();
    assert(t, expected_elapsed_time, actual_elapsed_time, expected_individual_threads, actual_individual_threads, expected_num_runs);
}

static void beforeEach(TestArg * t) {
    num_runs = 0;
    num_exited = 0;
    threadsSeen.clear();
}

int main(int argc, char * argv[]) {
    Tester t("ETasker Tests", nullptr, beforeEach, nullptr, nullptr);

    signal(SIGKILL, sig_handler1);
    // struct sigaction act = { 0 };
    // act.sa_flags = SA_SIGINFO;
    // act.sa_sigaction = &sig_handler1;
    // // if (sigaction(SIGKILL, &act, NULL) == -1 || sigaction(SIGSEGV, &act, NULL) == -1) {
    // if (sigaction(SIGKILL, &act, NULL) == -1) {
    //     std::string msg("Failed to configure sig action and sig handler.\n");
    //     loggerPrintf(LOGGER_INFO, "%s\n", msg.c_str());
    //     throw std::runtime_error(msg);
    // }

    t.addTest(testETasker);
    t.addTest(testETaskerBursty);
    t.addTest(testETaskerThreadLimit);
    t.addTest(testETaskerThreadLimitBursty);
    t.addTest(testETaskerThreadLimitPastLimit);
    // t.addTest(testETaskerThreadLimitFixed);
    t.addTest(testETaskerThreadLimitFixedBursty);
    // t.addTest(testETaskerThreadLimitFixedPastLimit);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}