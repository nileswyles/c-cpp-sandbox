#!/bin/sh

TEST_ARG=""
DEFINES=""
LOG_LEVEL=0
DEBUG=""
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES-D $2 "; shift 2 ;;
		-g) DEBUG="-g "; shift ;;
		*) TEST_ARG=$@; break;;
	esac
done

ROOT_DIR="."

SRC_FILES="
-s $ROOT_DIR/lib/test/matrix_test.cpp
-s $ROOT_DIR/lib/test/tester.cpp
-s $ROOT_DIR/lib/datastructures/array.cpp
-s $ROOT_DIR/lib/iostream/estream.cpp
-s $ROOT_DIR/lib/iostream/reader_task.cpp
"

CMD="$ROOT_DIR/build_scripts/build_common.sh -n matrix_test $SRC_FILES -l $LOG_LEVEL $DEBUG$DEFINES$TEST_ARG"
echo "\t"$CMD
exec $CMD