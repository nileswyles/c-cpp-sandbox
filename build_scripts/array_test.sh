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
-DGLOBAL_LOGGER_LEVEL=$LOG_LEVEL
-DLOGGER_LEVEL=$LOG_LEVEL
"

ROOT_DIR="."

# Standardize this
QUOTE_INCLUDE_ROOT=$ROOT_DIR/lib

SRC_FILES="
$ROOT_DIR/lib/test/array_test.cpp
$ROOT_DIR/lib/test/tester.cpp
$ROOT_DIR/lib/reader/reader.cpp
$ROOT_DIR/lib/array.cpp
"

mkdir $ROOT_DIR/out
TEST_PATH=$ROOT_DIR/out/array_test.out
rm $TEST_PATH
g++ $SRC_FILES -iquote $QUOTE_INCLUDE_ROOT $DEFINES -std=c++20 -o $TEST_PATH
exec $TEST_PATH $TEST_ARG