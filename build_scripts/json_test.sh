#!/bin/sh
ROOT_DIR="."

SRC_FILES="
$ROOT_DIR/lib/test/json_test.cpp
$ROOT_DIR/lib/json/json.cpp
"

# see logger.h for log level values
DEFINES="
-DLOGGER_LEVEL=$1
"

TEST_PATH=$ROOT_DIR/out/json_test.out

g++ $SRC_FILES -iquote $ROOT_DIR/lib -iquote $ROOT_DIR/lib/json $DEFINES -std=c++23 -o $TEST_PATH

exec $TEST_PATH