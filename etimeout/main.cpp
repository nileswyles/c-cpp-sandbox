#include <unistd.h>
#include <stdlib.h>
#include <signal.h>
#include <poll.h>
#include <stdbool.h>
#include <time.h>
#include <stdio.h>
#include <spawn.h>
#include <string.h>

#include <string>

#include "logger.h"

#define APP_NAME "etimeout"
#define VERSION "0.0.1"
#define MAX_ARGS 64

static int child_process_available = -1;
static int child_process_code = -1;

void logArgs(size_t argc, char * argv[]) {
    loggerExec(LOGGER_DEBUG,
        for (size_t i = 0; i < argc; i++) {
            printf("\t%s\n", argv[i]);
        }
    );
}

void logProcessArgs(size_t argc, char * argv[], char *envp[]) {
    // TODO: logger debug environment variables
    loggerPrintf(LOGGER_DEBUG, "ARGV: \n");
    logArgs(argc, argv);

    loggerPrintf(LOGGER_DEBUG, "ENVP: \n");
    loggerExec(LOGGER_DEBUG,
        char * cur = nullptr;
        size_t i = 0;
        while(i == 0 || cur != nullptr) {
            cur = envp[i++];
            if (cur != nullptr) {
                printf("\t%s\n", envp[i]);
            }
        }
    );
}

void logChildProcessArgs(size_t argc, char * argv[]) {
    loggerPrintf(LOGGER_DEBUG, "CHILD ARGV: \n");
    logArgs(argc, argv);
}

void printHelp() {
    std::string title_string(APP_NAME);
    title_string += " ";
    title_string += VERSION;
    printf("%s\n", title_string.c_str());
    printf("Must contain at least 2 positional arguments.\n");
    printf("\nRequired: \n");
    printf("\t<timeout_in_seconds> <cmd>\n");
    printf("Optional: \n");
    printf("\t<cmd_arguments>\n\n");
}

std::string getEnvironmentShell(char * envp[]) {
    std::string shell("/bin/sh");
    char * cur = nullptr;
    size_t i = 0;
    while(i == 0 || cur != nullptr) {
        cur = envp[i++];
        if (cur != nullptr) {
            std::string key_match = "SHELL";
            size_t j = 0;
            bool found_key = false;
            std::string s(cur);
            for (auto c: s) {
                if (false == found_key) {
                    // match key
                    if (j < key_match.size() || key_match[j++] != c) {
                        break;
                    }
                    if ('=' == c) {
                        found_key = true;
                        shell.clear();
                    }
                } else {
                    // accumulate value
                    shell += c;
                }
            }
            if (true == found_key) {
                break;
            }
        }
    }
    return shell;
}

void onProcessExit(int result_code, void * arg) {
    loggerPrintf(LOGGER_DEBUG, "Process timed exited with code: %d.\n", result_code);
    child_process_code = result_code;
    child_process_available = 0;
}

int main(int int_argc, char * argv[], char * envp[]) {
    size_t argc = static_cast<size_t>(int_argc);

    printf("argc: %lu\n", argc);
    if (3 > argc) {
        printHelp();
        exit(EXIT_FAILURE);
    }
    logProcessArgs(argc, argv, envp);

    int exit_code = 0;
    // esystem command with timeout.
    // run's command using fork, exec and polls... instead of wait_pid
    int timeout = atoi(argv[1]);

    // extract cmd and args
    char * path = argv[2];
    char ** cmd_argv = argv + 3;
    size_t cmd_argc = argc - 3; // because we want to include null string at end? lol...
    logChildProcessArgs(cmd_argc, cmd_argv);

    const char * shell = getEnvironmentShell(envp).c_str();
    loggerPrintf(LOGGER_DEBUG, "SHELL: %s\n", shell);

    // build shell args
    char * sh_argv[MAX_ARGS] = {};
    sh_argv[0] = const_cast<char *>("/bin/bash");
    sh_argv[1] = const_cast<char *>("-c");
    std::string cmd("\"");
    cmd += path;
    for (size_t i = 0; i < cmd_argc; i++) {
        cmd += ' ';
        cmd += cmd_argv[i];
    }
    cmd += '"';
    sh_argv[2] = const_cast<char *>(cmd.c_str());
    // char cmd[64] = {};
    // cmd[0] = '"';
    // size_t x = 1;
    // memcpy(cmd + x, path, strlen(path));
    // x += strlen(path);
    // for (size_t i = 0; i < cmd_argc; i++) {
    //     cmd[x++] = ' ';
    //     size_t len = strlen(cmd_argv[i]);
    //     memcpy(cmd + x, cmd_argv[i], len);
    //     x += len;
    // }
    // cmd[x] = '\"';
    // sh_argv[2] = cmd;
    loggerPrintf(LOGGER_DEBUG, "SH ARGV: \n");
    logArgs(cmd_argc + 4, sh_argv);

    // fork and forward sigs to child process
    if (signal(SIGCHLD, SIG_IGN) == SIG_ERR) {
        loggerPrintf(LOGGER_ERROR, "Failed to configure signals.\n");
        exit(EXIT_FAILURE);
    }
    int result = on_exit(onProcessExit, nullptr);
    if (result != 0) {
        loggerPrintf(LOGGER_ERROR, "Failed to register on_exit function handler.\n");
        exit(EXIT_FAILURE); 
    }
    // exec
    struct timespec ts_start;
    clock_gettime(CLOCK_MONOTONIC, &ts_start);
    pid_t child_process_id;
    posix_spawn_file_actions_t actions;
    posix_spawn_file_actions_init(&actions);
    posix_spawnattr_t attr;
    posix_spawnattr_init(&attr);
    // TODO: execvpe vs execve
    // result = posix_spawn(&child_process_id, shell, &actions, &attr, sh_argv, envp);
    result = posix_spawn(&child_process_id, sh_argv[0], nullptr, nullptr, sh_argv, envp);
    if (result == -1) { 
        loggerPrintf(LOGGER_ERROR, "Failed to execute program at %s with shell %s.\n", shell, shell);
        exit(EXIT_FAILURE); 
    } else if (child_process_id <= 0) { 
        loggerPrintf(LOGGER_ERROR, "Failed to fork from process.\n");
        exit(EXIT_FAILURE); 
    } else {
        loggerPrintf(LOGGER_DEBUG, "Forked process: %u\n", child_process_id);
    }
    // int child_fd = pidfd_open(child_process_id, PIDFD_NONBLOCK);
    // if (child_fd == -1) { exit(EXIT_FAILURE); }

    // my wait
    child_process_available = 1;
    bool has_timed_out = false;
    // struct pollfd pollfd;
    // pollfd.fd = child_fd;
    // pollfd.events = POLLIN;
    struct timespec ts;
    #define POLL_TIMEOUT 0 // return immediately
    while (child_process_available && false == has_timed_out) {
        sleep(1);
        clock_gettime(CLOCK_MONOTONIC, &ts);
        if (ts.tv_sec >= ts_start.tv_sec + timeout) {
            loggerPrintf(LOGGER_DEBUG, "Child process timed out.\n");
            kill(child_process_id, SIGKILL);
            has_timed_out = true;
            exit_code = 177;
        }
        // TODO: read about polling
        // might not need polling...
        // child_process_available = poll(&pollfd, 1, POLL_TIMEOUT); 
        // if (child_process_available == -1) { exit(EXIT_FAILURE); }
    }
    if (child_process_available == 0) {
        // process terminated normally, get exit code somehow.
        exit_code = child_process_code;
    }

    return exit_code;
}