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

if [ -z $WYLESLIBS_WORKSPACE_ROOT_DIR ]; then
	WYLESLIBS_WORKSPACE_ROOT_DIR=`pwd`
fi

SRC_FILES="
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/test/file_manager_test.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/test/tester.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/estream/byteestream.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/estream/istreamestream.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/datastructures/array.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/file/file.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/file/stream_factory.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/string_format.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/ecal.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/etime.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/cmder.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/logger_config_default.cpp
-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/paths.cpp
"

# ! IMPORTANT - 
# 	to run the GCS file manager test, set WYLESLIBS_GCS_BUILD=1
if [ -n "$WYLESLIBS_GCS_BUILD" ]; then
	GCS_ARGS=`$WYLESLIBS_WORKSPACE_ROOT_DIR/build_scripts/generate_gcs_arguments.sh`
	DEFINES="$DEFINES -D WYLESLIBS_GCS_BUILD=1 "
	SRC_FILES="$SRC_FILES -s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/file/file_gcs.cpp
		-s $WYLESLIBS_WORKSPACE_ROOT_DIR/lib/file/stream_factory_gcs.cpp
	"
fi

CMD="$WYLESLIBS_WORKSPACE_ROOT_DIR/build_scripts/build_common.sh -n file_manager_test $SRC_FILES $GCS_ARGS --log $LOG_LEVEL $DEFINES$TEST_ARG"
echo "    "$CMD
exec $CMD