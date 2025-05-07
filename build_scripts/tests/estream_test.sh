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
	WYLESLIBS_BUILD_ROOT_DIR=`pwd`
fi

# $WYLESLIBS_BUILD_ROOT_DIR/lib/json/json_mapper.cpp
SRC_FILES="
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/test/estream_test.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/byteestream.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/istreamestream.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/datastructures/array.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/test/tester.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/stream_factory.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/string_format.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/ecal.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/etime.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/cmder.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/default_logger_config.cpp
"

CMD="$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/build_common.sh -n estream_test $SRC_FILES --log $LOG_LEVEL $DEFINES$TEST_ARG"
echo "    "$CMD
exec $CMD