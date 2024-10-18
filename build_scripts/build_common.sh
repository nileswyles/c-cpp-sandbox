#!/bin/sh

NAME="program"
SRC_FILES=""
DEFINES=""
LD_FLAGS=""
LOG_LEVEL=0
PROGRAM_ARG=""
DEBUG=""
while true; do
	case "$1" in
        -n|--name) NAME="$2"; shift 2 ;;
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-s|--source) SRC_FILES="$SRC_FILES$2 "; shift 2 ;;
		-f) LD_FLAGS="$LD_FLAGS-l$2 "; shift 2 ;;
		-D) DEFINES="$DEFINES-D $2 "; shift 2 ;;
		-g) DEBUG="-g "; shift ;;
		# --) echo "PROGRAM ARG: $@"; break ;;
		*) PROGRAM_ARG=$@; break;;
	esac
done

# see logger.h for log level values
DEFINES=$DEFINES"-D GLOBAL_LOGGER_LEVEL=$LOG_LEVEL -D LOGGER_LEVEL=$LOG_LEVEL "

# TODO: I think I left this here because need would like to support different cwd paths eventually? (other than project root)
ROOT_DIR="."

# Standardize this
QUOTE_INCLUDE_ROOT=$ROOT_DIR/lib

mkdir $ROOT_DIR/out 2> /dev/null
PROGRAM_PATH=$ROOT_DIR/out/$NAME.out
rm $PROGRAM_PATH 2> /dev/null

if [ -z $CXX_COMPILER ]; then
	CXX_COMPILER="g++"
fi

REMOVED_WARNING_FLAGS="-Wno-unused-variable
-Wno-unused-parameter
-Wno-unused-function
"

echo "\n~Build: "
BUILD_CMD="$CXX_COMPILER $DEBUG$SRC_FILES-iquote $QUOTE_INCLUDE_ROOT -iquote $ROOT_DIR/http_test $DEFINES$LD_FLAGS-std=c++20 -Wall -Wextra -Wpedantic $REMOVED_WARNING_FLAGS -o $PROGRAM_PATH"
echo "\t$BUILD_CMD"
eval $BUILD_CMD

echo "\n~Executing Program: "
EXEC_CMD="$PROGRAM_PATH $PROGRAM_ARG"
echo "\t$EXEC_CMD"
eval $EXEC_CMD
