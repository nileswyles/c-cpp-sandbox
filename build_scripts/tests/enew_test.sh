#!/bin/sh

LOG_LEVEL=0
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-t|--test) TEST_ARG="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES -D$2"; shift 2 ;;
		# --) shift; break ;;
		*) break;;
	esac
done

# see logger.h for log level values
DEFINES=$DEFINES"
-DDYNAMIC_MEMORY_SIZE=16
-DGLOBAL_LOGGER_LEVEL=$LOG_LEVEL
-DLOGGER_LEVEL=$LOG_LEVEL
"

ROOT_DIR="."

SRC_FILES="
$ROOT_DIR/lib/test/enew_test.cpp
$ROOT_DIR/lib/enew.cpp
$ROOT_DIR/lib/memory/emalloc.c
$ROOT_DIR/lib/memory/heap.c
"

mkdir $ROOT_DIR/out 2> /dev/null
TEST_PATH=$ROOT_DIR/out/enew_test.out

g++ $SRC_FILES -iquote $ROOT_DIR/lib -iquote $ROOT_DIR/lib/memory $DEFINES -std=c++23 -o $TEST_PATH

exec $TEST_PATH