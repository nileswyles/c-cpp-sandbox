#!/bin/sh

LOG_LEVEL=0
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-p|--program) PROGRAM_ARG="$2"; shift 2 ;;
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
$ROOT_DIR/lib/json/parser/json_parser.cpp
$ROOT_DIR/lib/json/json_mapper.cpp
$ROOT_DIR/lib/json/parser/json_object.cpp
$ROOT_DIR/lib/json/parser/json_array.cpp
$ROOT_DIR/cpp_http/http.cpp
$ROOT_DIR/cpp_http/main.cpp
$ROOT_DIR/lib/reader/reader.cpp
$ROOT_DIR/lib/server.c
"

# LD_FLAGS="-lcrypto"
LD_FLAGS=""

PROGRAM_PATH=$ROOT_DIR/out/http-server.out
rm $PROGRAM_PATH
g++ $SRC_FILES -iquote $QUOTE_INCLUDE_ROOT -iquote $QUOTE_INCLUDE_ROOT/cpp_http $LD_FLAGS $DEFINES -Wno-pointer-arith -std=c++23 -o $PROGRAM_PATH
# exec $PROGRAM_PATH $PROGRAM_ARG
exec $PROGRAM_PATH 127.0.0.1 8080