#!/bin/sh

TEST_ARG=""
DEFINES=""
LOG_LEVEL=0
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES -D $2 "; shift 2 ;;
		*) TEST_ARG=$@; break;;
	esac
done

ROOT_DIR="."

# $ROOT_DIR/lib/json/json_mapper.cpp
SRC_FILES="
-s $ROOT_DIR/lib/test/json_test.cpp
-s $ROOT_DIR/lib/parser/json/json_parser.cpp
-s $ROOT_DIR/lib/parser/json/json_mapper.cpp
-s $ROOT_DIR/lib/parser/json/json_object.cpp
-s $ROOT_DIR/lib/parser/json/json_array.cpp
-s $ROOT_DIR/lib/test/tester.cpp
-s $ROOT_DIR/lib/iostream/iostream.cpp
-s $ROOT_DIR/lib/iostream/reader_task.cpp
"

CMD="$ROOT_DIR/build_scripts/build_common.sh -n json_test $SRC_FILES -l $LOG_LEVEL $DEFINES$TEST_ARG"
echo $CMD
exec $CMD