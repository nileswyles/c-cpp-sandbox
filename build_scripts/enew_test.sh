#!/bin/sh
ROOT_DIR="."

SRC_FILES="
$ROOT_DIR/lib/test/enew_test.cpp
$ROOT_DIR/lib/enew.cpp
$ROOT_DIR/lib/memory/emalloc.c
$ROOT_DIR/lib/memory/heap.c
"

# see logger.h for log level values
DEFINES="
-DDYNAMIC_MEMORY_SIZE=16
-DGLOBAL_LOGGER_LEVEL=$1
"

mkdir $ROOT_DIR/out
TEST_PATH=$ROOT_DIR/out/enew_test.out

g++ $SRC_FILES -iquote $ROOT_DIR/lib -iquote $ROOT_DIR/lib/memory $DEFINES -std=c++23 -o $TEST_PATH

exec $TEST_PATH