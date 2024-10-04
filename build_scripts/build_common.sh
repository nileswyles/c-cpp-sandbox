#!/bin/sh

NAME="something"
SRC_FILES=""
DEFINES=""
LD_FLAGS=""
LOG_LEVEL=0
while true; do
	case "$1" in
        -n|--name) NAME="$2"; shift 2 ;;
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-t|--test) TEST_ARG="$2"; shift 2 ;;
		-s|--source) SRC_FILES="$SRC_FILES $2"; shift 2 ;;
		-l) LD_FLAGS="$LD_FLAGS -l$2"; shift 2 ;;
		-D) DEFINES="$DEFINES -D$2"; shift 2 ;;
		# --) shift; break ;;
		*) break;;
	esac
done

# see logger.h for log level values
DEFINES=$DEFINES"
-DGLOBAL_LOGGER_LEVEL=$LOG_LEVEL
-DLOGGER_LEVEL=$LOG_LEVEL
"

ROOT_DIR="."

# Standardize this
QUOTE_INCLUDE_ROOT=$ROOT_DIR/lib

mkdir $ROOT_DIR/out 2> /dev/null
TEST_PATH=$ROOT_DIR/out/$NAME.out
rm $TEST_PATH 2> /dev/null
g++ $SRC_FILES -iquote $QUOTE_INCLUDE_ROOT $DEFINES $LD_FLAGS -std=c++20 -o $TEST_PATH
exec $TEST_PATH $TEST_ARG