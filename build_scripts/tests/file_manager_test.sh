#!/bin/bash

TEST_ARG=""
DEFINES=""
LOG_LEVEL=0
GCS_INSTALLED_FROM_VCPKG_REPO=""
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES-D $2 "; shift 2 ;;
		--gcs_installed_from_vcpkg_repo) GCS_INSTALLED_FROM_VCPKG_REPO="--gcs_installed_from_vcpkg_repo"; shift 1 ;;
		*) TEST_ARG=$@; break;;
	esac
done

if [ -z $WYLESLIBS_BUILD_ROOT_DIR ]; then
	WYLESLIBS_BUILD_ROOT_DIR="."
fi

SRC_FILES="
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/test/file_manager_test.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/test/tester.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/estream.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/datastructures/array.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/reader_task.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/stream_factory.cpp
"

# ! IMPORTANT - 
# 	to run the GCS file manager test, set FILE_MANAGER_GCS_TEST=1
if [ -n "$FILE_MANAGER_GCS_TEST" ]; then
	GCS_ARGS=`$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/generate_gcs_arguments.sh $GCS_INSTALLED_FROM_VCPKG_REPO`
	DEFINES="$DEFINES -D FILE_MANAGER_GCS_TEST=1 "
	SRC_FILES="$SRC_FILES -s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file_gcs.cpp
		-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/stream_factory_gcs.cpp
	"
fi

CMD="$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/build_common.sh -n file_manager_test $SRC_FILES $GCS_ARGS --log $LOG_LEVEL $DEFINES$TEST_ARG"
echo "\t"$CMD
exec $CMD