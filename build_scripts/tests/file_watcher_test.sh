#!/bin/bash

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

if [ -z $WYLESLIBS_BUILD_ROOT_DIR ]; then
	WYLESLIBS_BUILD_ROOT_DIR="."
fi

SRC_FILES="
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/test/file_watcher_test.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file_watcher.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/test/tester.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/datastructures/array.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/estream.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/string-format.cpp
"

CMD="$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/build_common.sh -n file_watcher_test $SRC_FILES --log $LOG_LEVEL $DEFINES$TEST_ARG"
echo "\t"$CMD
exec $CMD