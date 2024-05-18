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
-DLOGGER_LEVEL=$LOG_LEVEL
"

ROOT_DIR="."

# Standardize this
QUOTE_INCLUDE_ROOT=$ROOT_DIR/lib

# oh yeah.... lol


TESTER_SRC_FILES="
$ROOT_DIR/lib/c/array.c
$ROOT_DIR/lib/test/tester.c
"
TESTER_PATH=$ROOT_DIR/out/reader_tester.o
rm $TESTER_PATH
gcc $TESTER_SRC_FILES $TEST_C_LIB_PATH -iquote $QUOTE_INCLUDE_ROOT -iquote $QUOTE_INCLUDE_ROOT/c $DEFINES -Wno-pointer-arith -std=c++23 -o $TEST_PATH
exec $TEST_PATH $TEST_ARG

# $ROOT_DIR/lib/json/json_mapper.cpp
SRC_FILES="
$ROOT_DIR/lib/test/reader_test.cpp
$ROOT_DIR/lib/reader/reader.cpp
"

TEST_PATH=$ROOT_DIR/out/reader_test.out
rm $TEST_PATH
g++ $SRC_FILES $TEST_C_LIB_PATH -iquote $QUOTE_INCLUDE_ROOT $DEFINES -Wno-pointer-arith -std=c++23 -o $TEST_PATH
exec $TEST_PATH $TEST_ARG