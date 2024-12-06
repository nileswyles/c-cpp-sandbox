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

static size_t numRuns = 0;
static size_t numExited = 0;
static std::set<pthread_t> threadsSeen;

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
            loggerPrintf(LOGGER_DEBUG, "run called\n");
            // numRuns++;
        }
        void onExit() {
            sleep(5);
            loggerPrintf(LOGGER_DEBUG, "onExit called\n");
            // numExited++;
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

// void sig_handler1(int sig, siginfo_t * info, void * context) {
//     pthread_t pthread = pthread_self();
//     if (ETasker::thread_specific_sig_handlers.contains(pthread)) {
//         ETasker * etasker = ETasker::thread_specific_sig_handlers[pthread];
//         etasker->threadSigAction(sig, info, context);
//     } else {
//         raise(sig);
//     }
// }

void testETasker(TestArg * t) {
    ETasker lol(SIZE_MAX, 17, false);
    lol.startTimeoutThread();

    struct timespec prior;
    clock_gettime(CLOCK_MONOTONIC, &prior);

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
    size_t expectedNumRuns = get_nprocs() * procs_mul;
    loggerPrintf(LOGGER_DEBUG, "nprocs: %u, procs_mul: %lu\n", get_nprocs(), procs_mul);
    ESharedPtr<TestETask> ptr(new TestETask());
    for (size_t i = 0; i < expectedNumRuns; i++) {
        lol.run(ptr);
    }

    while (expectedNumRuns != numExited) {
        for (auto i: ETasker::thread_specific_sig_handlers) {
            threadsSeen.insert(i.first);
        }
        usleep(270); // lol... flaky? yeah 
    }

    struct timespec after;
    clock_gettime(CLOCK_MONOTONIC, &after);

    // size_t sleep_time = 16 * procs_mul;
    size_t sleep_time = 16;
    // should be ~15 seconds, give an extra second just because...
    //      then test all config permutations.
    size_t numIndividualThreads = threadsSeen.size();
    if (static_cast<uint64_t>(after.tv_sec - prior.tv_sec) > sleep_time 
        || numRuns != expectedNumRuns 
        || numIndividualThreads != expectedNumRuns) {
        t->fail = true;
    }
}

static void beforeEach(TestArg * t) {
    numRuns = 0;
    numExited = 0;
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
    // t.addTest(testETaskerThreadLimit);
    // t.addTest(testETaskerThreadLimitFixed);
    // t.addTest(testETaskerThreadLimitBursty);
    // t.addTest(testETaskerThreadLimitFixedBursty);

    bool passed = false;
    if (argc > 1) {
        loggerPrintf(LOGGER_DEBUG, "argc: %d, argv[0]: %s\n", argc, argv[1]);
        passed = t.run(argv[1]);
    } else {
        passed = t.run(nullptr);
    }

    return passed ? 0 : 1;
}