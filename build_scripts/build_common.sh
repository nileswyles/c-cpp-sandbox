#!/bin/bash

if [ -z $WYLESLIBS_BUILD_ROOT_DIR ]; then
	WYLESLIBS_BUILD_ROOT_DIR=`pwd`
fi

NAME="program"
LD_FLAGS=""
LIB_SEARCH_PATHS=""
INCLUDE_FILES=""
DEFINES=""
SRC_FILES=""
LOG_LEVEL=0
RUN_FROM="$WYLESLIBS_BUILD_ROOT_DIR"
OUTPUT_DIR="$WYLESLIBS_BUILD_ROOT_DIR/out"
PROGRAM_ARG=""
DEBUG=""
while true; do
	case "$1" in
        -n|--name) NAME="$2"; shift 2 ;;
		--log) LOG_LEVEL="$2"; shift 2 ;;
		-l) LD_FLAGS="$LD_FLAGS-l $2 "; shift 2 ;;
		-l*) LD_FLAGS="$LD_FLAGS$1 "; shift 1 ;;
		-L) LIB_SEARCH_PATHS="$LIB_SEARCH_PATHS-L $2 "; shift 2 ;;
		-L*) LIB_SEARCH_PATHS="$LIB_SEARCH_PATHS$1 "; shift 1 ;;
		-I|--include) INCLUDE_FILES="$INCLUDE_FILES-I $2 "; shift 2 ;;
		-I*) INCLUDE_FILES="$INCLUDE_FILES$1 "; shift 1 ;;
		-D) DEFINES="$DEFINES-D $2 "; shift 2 ;;
		-D*) DEFINES="$DEFINES$1 "; shift 1 ;;
		-s|--source) SRC_FILES="$SRC_FILES$2 "; shift 2 ;;
		-r|--run-from) RUN_FROM="$2"; shift 2 ;;
		-o|--output-dir) OUTPUT_DIR="$2"; shift 2 ;;
		-g) DEBUG="-g "; shift ;;
		# --) echo "PROGRAM ARG: $@"; break ;;
		*) PROGRAM_ARG=$@; break;;
	esac
done

# see logger.h for log level values
# TODO: remember to remove need to set LOGGER_LEVEL... this was a stop-gap to support legacy stuff, need to update all files.
#		And actually don't need to set GLOBAL_LOGGER_LEVEL in each file.
DEFINES=$DEFINES"-D GLOBAL_LOGGER_LEVEL=$LOG_LEVEL -D LOGGER_LEVEL=$LOG_LEVEL "

# Standardize this
INCLUDE_ROOT=$WYLESLIBS_BUILD_ROOT_DIR/lib

if [ $(cut -b 1 - <<< $OUTPUT_DIR) != "/" ]; then
	OUTPUT_DIR="$(pwd)/$OUTPUT_DIR"
fi
mkdir $OUTPUT_DIR 2> /dev/null
PROGRAM_PATH="$OUTPUT_DIR/$NAME.out"
rm $PROGRAM_PATH 2> /dev/null

if [ -z $CXX_COMPILER ]; then
	CXX_COMPILER="g++"
fi

REMOVED_WARNING_FLAGS="-Wno-unused-variable -Wno-unused-parameter -Wno-unused-function -Wno-reorder -Wno-deprecated-declarations"

echo "
~Build: "
BUILD_CMD="$CXX_COMPILER $DEBUG$SRC_FILES-I $INCLUDE_ROOT $INCLUDE_FILES$LIB_SEARCH_PATHS$DEFINES$LD_FLAGS-std=c++20 -Wall $REMOVED_WARNING_FLAGS -o $PROGRAM_PATH"
echo "    $BUILD_CMD"
$BUILD_CMD

echo "
~Executing Program: "
EXEC_CMD="cd $RUN_FROM && $PROGRAM_PATH $PROGRAM_ARG"
echo "    $EXEC_CMD"
bash -c "$EXEC_CMD"