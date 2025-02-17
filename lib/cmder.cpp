#include "cmder.h"

#include "file/file.h"

#include <stdlib.h>
#include <stdio.h>
#include <unistd.h>

using namespace WylesLibs;
using namespace WylesLibs::File;

#define ESYSTEM_FILE_PREFIX "wyleslibs_esystem"

extern std::string WylesLibs::esystem(const char * cmd, SharedArray<char *> args) {
    static FileManager file_manager;
    // TODO: because I can't read from stdout directly? 
    //      tell, seek on stdout didn't work and glibc doesn't otherwise provide an explicit way for configuring stdout (besides setting the global variable).
    //      dup?
    //      would have been nice to configure an in-mem buffer, second best solution is to tell and seek, then there's this solution.

    //      revisit eventually and determine whether or not to use something other than system.
    std::string tmp_out = WylesLibs::format("/tmp/{s}_{d}.out", ESYSTEM_FILE_PREFIX, random());
    std::string tmp_err = WylesLibs::format("/tmp/{s}_{d}.err", ESYSTEM_FILE_PREFIX, rand());

    // again, this is tedious.
    std::string cmd_with_args = WylesLibs::format("/bin/bash -c \"{s} {s} 1> {s} 2> {s}\"", cmd, args.toString().c_str(), tmp_out.c_str(), tmp_err.c_str());
    loggerPrintf(LOGGER_DEBUG, "cmd_with_args: %s\n", cmd_with_args.c_str());
    int ret = system(cmd_with_args.c_str());
    if (ret) {
        loggerPrintf(LOGGER_INFO, "Error executing command: %s, errno: %i\n", cmd_with_args.c_str(), errno);
    }

    std::string out_s;
    try {
        if (file_manager.exists(tmp_err) && file_manager.stat(tmp_err)) {
            loggerPrintf(LOGGER_INFO, "Message received via Standard Error while executing command: %s\n", cmd_with_args.c_str());
            loggerPrintf(LOGGER_INFO, "Standard Error: %s\n", file_manager.read<std::string>(tmp_err).c_str());
        } 
        if (file_manager.exists(tmp_out) == 0) {
            loggerPrintf(LOGGER_INFO, "Error executing command: %s, Standard Out file does not exist.\n", cmd_with_args.c_str());
        } else {
            out_s = file_manager.read<std::string>(tmp_out);
        }
    } catch(std::exception& e) {
        loggerPrintf(LOGGER_INFO, "Exception: %s\n", e.what());
    }
    try {
        file_manager.remove(tmp_out);
    } catch(std::exception& e) {
        loggerPrintf(LOGGER_INFO, "Error while deleting file at path: %s\n", tmp_out.c_str());
        loggerPrintf(LOGGER_INFO, "Exception: %s\n", e.what());
    }
    try {
        file_manager.remove(tmp_err);
    } catch(std::exception& e) {
        loggerPrintf(LOGGER_INFO, "Error while deleting file at path: %s\n", tmp_out.c_str());
        loggerPrintf(LOGGER_INFO, "Exception: %s\n", e.what());
    }

    return out_s;
}