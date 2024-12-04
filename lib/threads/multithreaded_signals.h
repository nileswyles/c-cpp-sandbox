#ifndef WYLESLIBS_MULTITHREADED_SIGNALS
#define WYLESLIBS_MULTITHREADED_SIGNALS

#include <map>

#include <signal.h>

namespace WylesLibs {
    typedef void(SigHandler*)(int, siginfo_t *, void *);

    extern map<pthread_t, SigHandler> thread_specific_sig_handlers;

    extern registerThreadSigActionHandler(SigHandler handler);
};

#endif