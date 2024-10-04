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

# $ROOT_DIR/lib/json/json_mapper.cpp
SRC_FILES="
$ROOT_DIR/lib/test/iostream_test.cpp
$ROOT_DIR/lib/iostream/iostream.cpp
$ROOT_DIR/lib/iostream/reader_task.cpp
$ROOT_DIR/lib/test/tester.cpp
"

LD_FLAGS="
-lssl
-lcrypto
"

mkdir $ROOT_DIR/out 2> /dev/null
TEST_PATH=$ROOT_DIR/out/iostream_test.out
rm $TEST_PATH
g++ $SRC_FILES -iquote $QUOTE_INCLUDE_ROOT -iquote $QUOTE_INCLUDE_ROOT/iostream $LD_FLAGS $DEFINES -Wno-pointer-arith -std=c++23 -o $TEST_PATH
exec $TEST_PATH $TEST_ARG