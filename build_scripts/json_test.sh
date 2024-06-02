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
$ROOT_DIR/lib/test/json_test.cpp
$ROOT_DIR/lib/json/parser/json_parser.cpp
$ROOT_DIR/lib/json/json_mapper.cpp
$ROOT_DIR/lib/json/parser/json_object.cpp
$ROOT_DIR/lib/json/parser/json_array.cpp
$ROOT_DIR/lib/c/array.c
$ROOT_DIR/lib/test/tester.cpp
$ROOT_DIR/lib/reader/reader.cpp
"

TEST_PATH=$ROOT_DIR/out/json_test.out
rm $TEST_PATH
g++ $SRC_FILES -iquote $QUOTE_INCLUDE_ROOT -iquote $ROOT_DIR/lib/json $DEFINES -Wno-pointer-arith -std=c++23 -o $TEST_PATH
exec $TEST_PATH $TEST_ARG