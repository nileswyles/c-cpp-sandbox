#!/bin/sh

TEST_ARG=""
DEFINES=""
LOG_LEVEL=0
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES-D $2 "; shift 2 ;;
		*) PROGRAM_ARG=$@; break;;
	esac
done

DEFINES="$DEFINES-D WYLESLIBS_SSL_ENABLED=1"

ROOT_DIR="."

SRC_FILES="
-s $ROOT_DIR/lib/parser/json/json_parser.cpp
-s $ROOT_DIR/lib/parser/json/json_mapper.cpp
-s $ROOT_DIR/lib/parser/json/json_object.cpp
-s $ROOT_DIR/lib/parser/json/json_array.cpp
-s $ROOT_DIR/lib/parser/keyvalue/parse.cpp
-s $ROOT_DIR/lib/web/http/http.cpp
-s $ROOT_DIR/http_test/main.cpp
-s $ROOT_DIR/http_test/controllers/example.cpp
-s $ROOT_DIR/http_test/services/example.cpp
-s $ROOT_DIR/lib/iostream/iostream.cpp
-s $ROOT_DIR/lib/web/server.c
-s $ROOT_DIR/lib/file_watcher.cpp
-s $ROOT_DIR/lib/web/http/http_file_watcher.cpp
-s $ROOT_DIR/lib/datastructures/array.cpp
"

LD_FLAGS="
-f ssl
-f crypto
"

CMD="$ROOT_DIR/build_scripts/build_common.sh -n http_server $SRC_FILES -l $LOG_LEVEL $LD_FLAGS $DEFINES$PROGRAM_ARG"
echo "\t"$CMD
exec $CMD