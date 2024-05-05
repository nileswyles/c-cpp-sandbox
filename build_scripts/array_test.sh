#!/bin/sh
ROOT_DIR="."
g++ $ROOT_DIR/lib/test/array_test.cpp -iquote $ROOT_DIR/lib -Wno-pointer-arith -DLOGGER_LEVEL=1 -std=c++23 -o $ROOT_DIR/out/array_test.out
