#include <stdio.h>
#include <stdint.h>
#include <signal.h>
#include <stdlib.h>

// (base) mysteres@sandbox:/mnt/storage/workspaces/c_fun$ ulimit -a
// real-time non-blocking time  (microseconds, -R) unlimited
// core file size              (blocks, -c) 0
// data seg size               (kbytes, -d) unlimited
// scheduling priority                 (-e) 0
// file size                   (blocks, -f) unlimited
// pending signals                     (-i) 94909
// max locked memory           (kbytes, -l) 3063476
// max memory size             (kbytes, -m) unlimited
// open files                          (-n) 1048576
// pipe size                (512 bytes, -p) 8
// POSIX message queues         (bytes, -q) 819200
// real-time priority                  (-r) 0
// stack size                  (kbytes, -s) 8192
// cpu time                   (seconds, -t) unlimited
// max user processes                  (-u) 94909
// virtual memory              (kbytes, -v) unlimited
// file locks                          (-x) unlimited

int sig = 0;

void sig_handler(int signum) {
    printf("Received signal %d\n", signum);
    sig = signum;
    // exit(signum);
}

int main() {
    signal(SIGSEGV, sig_handler);
    uint32_t blah = 8192000;
    uint8_t test2[blah];
    if (sig == 0) {
        // Does program restart after sig_handler is called?
        test2[blah + 7000000] = 0;
    }
    printf("VLA successful declaration\n");
    // uint8_t test[8192000] = {};
    printf("Static successful declaration\n");
    // moral of the story... use this feature sparingly...
    return 0;
}