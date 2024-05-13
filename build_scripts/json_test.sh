#!/bin/sh
ROOT_DIR="."

# Standardize this
QUOTE_INCLUDE_ROOT=$ROOT_DIR/lib

# lol, because c vs c++ warnings like pointer arith, etc... this is only for now... need to makefile eventually.
SRC_C="
$ROOT_DIR/lib/c/array.c
$ROOT_DIR/lib/test/tester.c
"
TEST_C_LIB_OBJ_PATH=$ROOT_DIR/out/json_test.o
TEST_C_LIB_PATH=$ROOT_DIR/out/json_test.a
rm $TEST_C_LIB_OBJ_PATH
rm $TEST_C_LIB_PATH
# gcc $SRC_C -iquote $QUOTE_INCLUDE_ROOT -std=c11 -c -o $TEST_C_LIB_OBJ_PATH
gcc $SRC_C -iquote $QUOTE_INCLUDE_ROOT -std=c11 -c $TEST_C_LIB_PATH

######

echo "YOU'RE DUMB"

SRC_FILES="
$ROOT_DIR/lib/test/json_test.cpp
$ROOT_DIR/lib/json/json.cpp
"
# see logger.h for log level values
DEFINES="
-DLOGGER_LEVEL=$1
"
TEST_PATH=$ROOT_DIR/out/json_test.out
rm $TEST_PATH
g++ $SRC_FILES $TEST_C_LIB_PATH -iquote $QUOTE_INCLUDE_ROOT -iquote $ROOT_DIR/lib/json $DEFINES -std=c++23 -o $TEST_PATH
exec $TEST_PATH