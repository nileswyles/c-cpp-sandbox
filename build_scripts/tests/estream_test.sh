#!/bin/sh

TEST_ARG=""
DEFINES=""
LOG_LEVEL=0
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES-D $2 "; shift 2 ;;
		*) TEST_ARG=$@; break;;
	esac
done

ROOT_DIR="."

# $ROOT_DIR/lib/json/json_mapper.cpp
SRC_FILES="
-s $ROOT_DIR/lib/test/estream_test.cpp
-s $ROOT_DIR/lib/estream/estream.cpp
-s $ROOT_DIR/lib/estream/reader_task.cpp
-s $ROOT_DIR/lib/test/tester.cpp
"

LD_FLAGS="
-l ssl
-l crypto
"

CMD="$ROOT_DIR/build_scripts/build_common.sh -n iostream_test $SRC_FILES --log $LOG_LEVEL $LD_FLAGS $DEFINES$TEST_ARG"
echo "\t"$CMD
exec $CMD