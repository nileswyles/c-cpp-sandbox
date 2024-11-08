#include <signal.h>

#include "logger.h"

static int status = 0;
void sig_handler(int sig, siginfo_t * info, void * context) {
    if (sig == SIGSEGV) {
        status = info->si_status;
        loggerPrintf(LOGGER_DEBUG, "SIGSEGV\n");
        raise(SIGKILL);
    }
}

int main() {
    struct sigaction act = { 0 };
    act.sa_flags = SA_SIGINFO;
    act.sa_sigaction = &sig_handler;
    if (sigaction(SIGSEGV, &act, NULL) == -1) {
        loggerPrintf(LOGGER_DEBUG, "Failed to configure sig action and sig handler.\n");
    }

    int * lol = NULL;
    *lol = 1;

    return 0;
}