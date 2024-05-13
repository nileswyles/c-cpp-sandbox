#!/bin/sh
ROOT_DIR="."

TEST_PATH=$ROOT_DIR/out/array_test.out

rm $TEST_PATH

g++ $ROOT_DIR/lib/test/array_test.cpp -iquote $ROOT_DIR/lib -DLOGGER_LEVEL=1 -std=c++23 -o $TEST_PATH

exec $TEST_PATH
