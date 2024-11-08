#!/bin/sh

# Generate a Project Buildsystem
#  cmake [<options>] -B <path-to-build> [-S <path-to-source>]
#  cmake [<options>] <path-to-source | path-to-existing-build>
# 
# Build a Project
#  cmake --build <dir> [<options>] [-- <build-tool-options>]
# 
# Install a Project
#  cmake --install <dir> [<options>]
# 
# Open a Project
#  cmake --open <dir>
# 
# Run a Script
#  cmake [-D <var>=<value>]... -P <cmake-script-file>
# 
# Run a Command-Line Tool
#  cmake -E <command> [<options>]
# 
# Run the Find-Package Tool
#  cmake --find-package [<options>]
# 
# Run a Workflow Preset
#  cmake --workflow <options>
# 
# View Help
#  cmake --help[-<topic>]

cmake -B build -S .
cmake --build build
