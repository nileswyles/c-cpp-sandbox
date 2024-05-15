#!/bin/sh
ROOT_DIR="."

# Standardize this
QUOTE_INCLUDE_ROOT=$ROOT_DIR/lib

# $ROOT_DIR/lib/json/json_mapper.cpp
SRC_FILES="
$ROOT_DIR/lib/test/json_test.cpp
$ROOT_DIR/lib/json/json_parser.cpp
$ROOT_DIR/lib/c/array.c
$ROOT_DIR/lib/test/tester.c
"
# see logger.h for log level values
DEFINES="
-DLOGGER_LEVEL=$1
"
TEST_PATH=$ROOT_DIR/out/json_test.out
rm $TEST_PATH
g++ $SRC_FILES $TEST_C_LIB_PATH -iquote $QUOTE_INCLUDE_ROOT -iquote $ROOT_DIR/lib/json $DEFINES -Wno-pointer-arith -std=c++23 -o $TEST_PATH
exec $TEST_PATH