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

# $ROOT_DIR/lib/json/json_mapper.cpp
SRC_FILES="
$ROOT_DIR/lib/test/reader_test.cpp
$ROOT_DIR/lib/reader/reader.cpp
$ROOT_DIR/lib/test/tester.cpp
"

mkdir $ROOT_DIR/out
TEST_PATH=$ROOT_DIR/out/reader_test.out
rm $TEST_PATH
g++ $SRC_FILES -iquote $QUOTE_INCLUDE_ROOT -iquote $QUOTE_INCLUDE_ROOT/reader $DEFINES -Wno-pointer-arith -std=c++23 -o $TEST_PATH
exec $TEST_PATH $TEST_ARG