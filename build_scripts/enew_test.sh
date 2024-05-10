#!/bin/sh
ROOT_DIR="."
SRC_FILES="
$ROOT_DIR/lib/test/enew_test.cpp
$ROOT_DIR/lib/enew.cpp
$ROOT_DIR/lib/memory/emalloc.c
$ROOT_DIR/lib/memory/heap.c
"

g++ $SRC_FILES -iquote $ROOT_DIR/lib -iquote $ROOT_DIR/lib/memory -DLOGGER_LEVEL=2 -std=c++23 -o $ROOT_DIR/out/enew_test.out
