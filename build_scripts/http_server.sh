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
$ROOT_DIR/lib/parser/json/json_parser.cpp
$ROOT_DIR/lib/parser/json/json_mapper.cpp
$ROOT_DIR/lib/parser/json/json_object.cpp
$ROOT_DIR/lib/parser/json/json_array.cpp
$ROOT_DIR/lib/parser/keyvalue/parse.cpp
$ROOT_DIR/lib/web/http/http.cpp
$ROOT_DIR/http_test/main.cpp
$ROOT_DIR/http_test/controllers/example.cpp
$ROOT_DIR/http_test/services/example.cpp
$ROOT_DIR/lib/reader/reader.cpp
$ROOT_DIR/lib/web/server.c
"

LD_FLAGS="
-lssl
-lcrypto
"
LD_FLAGS=""

PROGRAM_PATH=$ROOT_DIR/http_test/out/http-server.out
rm $PROGRAM_PATH
g++ $SRC_FILES -iquote $QUOTE_INCLUDE_ROOT -iquote $ROOT_DIR/http_test -I/home/floxnard7/openssl-3.3.0/include $LD_FLAGS $DEFINES -Wno-pointer-arith -std=c++23 -o $PROGRAM_PATH

cd http_test
exec out/http-server.out 