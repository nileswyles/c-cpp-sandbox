#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>

int sig = 0;

void sig_handler(int signum) {
    printf("Received signal %d\n", signum);
    sig = signum;
    exit(signum);
}

int main() {
    signal(SIGSEGV, sig_handler);
    uint32_t * blah = NULL;
    uint32_t test = -1;
    if (sig == 0) {
        // Does program restart after sig_handler is called?
        test = *blah;
    }
    printf("test %u\n", test);
    return 0;
}