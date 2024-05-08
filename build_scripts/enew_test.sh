#!/bin/sh
ROOT_DIR="."
g++ $ROOT_DIR/lib/test/enew_test.cpp $ROOT_DIR/lib/enew.cpp $ROOT_DIR/lib/memory/emalloc.c -iquote $ROOT_DIR/lib -iquote $ROOT_DIR/lib/memory -DLOGGER_LEVEL=1 -std=c++23 -o $ROOT_DIR/out/enew_test.out
