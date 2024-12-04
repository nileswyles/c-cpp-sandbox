#include "multithreaded_signals.h"

extern map<pthread_t, SigHandler> WylesLibs::thread_specific_sig_handlers = map<pthread_t, SigHandler>();

extern WylesLibs::registerThreadSigActionHandler(sig_handler handler) {
    thread_specific_sig_handlers[pthread_self()] = handler;
}