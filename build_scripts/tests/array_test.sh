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

SRC_FILES="
-s $ROOT_DIR/lib/test/array_test.cpp 
-s $ROOT_DIR/lib/test/tester.cpp 
-s $ROOT_DIR/lib/iostream/iostream.cpp 
-s $ROOT_DIR/lib/array.cpp
"

CMD="$ROOT_DIR/build_scripts/build_common.sh -n array_test $SRC_FILES -l $LOG_LEVEL $DEFINES$TEST_ARG"
echo $CMD
exec $CMD