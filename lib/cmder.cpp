#include "cmder.h"

#include <stdio.h>
#include <unistd.h>

using namespace WylesLibs;

// ! IMPORTANT - monitor. I think it's fine.
//      drop in the pond compared to the default 8MB stack size... Large files might require a different signature anyways.
#define IO_BUF_SIZE 32768
#define ESYSTEM_FILE_PREFIX "wyleslibs_esystem"

extern std::string WylesLibs::esystem(const char * cmd, SharedArray<char *> args) {
    FILE * tmp_stdout = stdout;
    FILE * tmp_stderr = stderr;
    char c_out[IO_BUF_SIZE] = {0};
    char c_err[IO_BUF_SIZE] = {0};

    // TODO: because I can't read from stdout directly? 
    //      tell, seek on stdout didn't work and glibc doesn't otherwise appear to provide any facilities to configuring stdout.
    //      would have been nice to configure a in-mem buffer, second best solution is to tell and seek, then there's this solution.

    //      revisit eventually and determine whether or not to use something other than system.
    std::string tmp_file_path = WylesLibs::format("/tmp/{s}_{d}", ESYSTEM_FILE_PREFIX, rand());
    std::string cmd_with_args = WylesLibs::format("/bin/bash -c \"{s} {s} > {s}\"", std::string(cmd), args.toString(), tmp_file_path);
    printf("cmd_with_args: %s\n", cmd_with_args.c_str());
    // stdout = fmemopen(c_out, IO_BUF_SIZE, "r+");
    // stderr = fmemopen(c_err, IO_BUF_SIZE, "r+");
    int ret = system(cmd_with_args.c_str()); // blocking...
    static FileManager * file_manager = new FileManager();
    // int ret = system("ls"); // blocking...
    if (ret) {
        loggerPrintf(LOGGER_INFO, "Error executing command: %s, errno: %i\n", cmd_with_args.c_str(), errno);
    }
    if (c_err[0] && c_err[IO_BUF_SIZE - 1] == 0) {
        loggerPrintf(LOGGER_INFO, "Message received via Standard Error while executing command: %s\n", cmd_with_args.c_str());
        loggerPrintf(LOGGER_INFO, "Standard Error: %s\n", c_err);
    } 
    std::string out_s;
    if (c_out[IO_BUF_SIZE - 1] != 0) {
        loggerPrintf(LOGGER_INFO, "Error executing command: %s, too much information in the output buffer.\n", cmd_with_args.c_str());
    } else {
        SharedArray<uint8_t> read_file_data = file_manager->read(test_file);
        out_s = std::string(c_out);
    }
    system(WylesLibs::format("rm {s}", tmp_file_path).c_str());
    // fclose(stdout);
    // stdout = tmp_stdout;
    // fclose(stderr);
    // stderr = tmp_stderr;
    printf("OUT: %s\n", out_s.c_str());

    return out_s;
}