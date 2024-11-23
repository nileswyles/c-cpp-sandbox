#!/bin/bash

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

DEFINES="$DEFINES-D WYLESLIBS_SSL_ENABLED=1 "

if [ -z $WYLESLIBS_BUILD_ROOT_DIR ]; then
	WYLESLIBS_BUILD_ROOT_DIR="."
fi

# this should work just fine until we have larger projects and require caching individual objects to speed up build times?
SRC_FILES="
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/parser/json/json_parser.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/parser/json/json_mapper.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/parser/json/json_object.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/parser/json/json_array.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/parser/keyvalue/parse.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/web/http/http.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/http_test/main.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/http_test/controllers/example.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/http_test/services/example.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/byteestream.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/istreamestream.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/reader_task.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/web/server.c
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file_watcher.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/web/http/http_file_watcher.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/datastructures/array.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/stream_factory.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file_gcs.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/stream_factory_gcs.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/string_format.cpp
"

INCLUDE_DIRS="-I $WYLESLIBS_BUILD_ROOT_DIR/http_test"

LD_FLAGS="
-l ssl
-l crypto
"

GCS_ARGS=`$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/generate_gcs_arguments.sh`

CMD="$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/build_common.sh -n http_server $SRC_FILES -r $WYLESLIBS_BUILD_ROOT_DIR/http_test --log $LOG_LEVEL $INCLUDE_DIRS $LD_FLAGS $GCS_ARGS $DEFINES$PROGRAM_ARG"
# TODO: revisit quoted strings and whitespace (nl, tabs, etc) for bash shell... Also, wtf is dash shell? zsh and bash I think are mostly identical for most basic things?
echo "\t"$CMD
exec $CMD




# google-cloud-cpp
# 	from pkg-config after building "and installing" using build_google.sh

# CFLAGS_CMD="pkg-config $PKG_NAMES --cflags"
# echo $CFLAGS_CMD
# GCS_CXXFLAGS=`$CFLAGS_CMD`
# GCS_CXXLDFLAGS=`pkg-config $PKG_NAMES --libs-only-L`

# # cflags
# GCS_CXXFLAGS="pkg-config $PKG_NAMES --cflags"
# -DNOMINMAX 
# -I$WYLESLIBS_BUILD_ROOT_DIR/google-cloud-cpp/cmake-out/google/cloud/storage/../../include 
# -I$WYLESLIBS_BUILD_ROOT_DIR/google-cloud-cpp/cmake-out/google/cloud/../../include 
# -I/usr/include/x86_64-linux-gnu

# # -l
# GCS_LIBS=`pkg-config $PKG_NAMES --libs-only-l`
# -lgoogle_cloud_cpp_storage 
# -lcrc32c 
# -labsl_cord 
# -labsl_str_format_internal 
# -lgoogle_cloud_cpp_rest_internal 
# -lgoogle_cloud_cpp_common 
# -labsl_bad_optional_access 
# -labsl_time 
# -labsl_civil_time 
# -labsl_strings 
# -labsl_strings_internal 
# -labsl_base 
# -labsl_spinlock_wait 
# -labsl_int128 
# -labsl_throw_delegate 
# -labsl_time_zone 
# -labsl_bad_variant_access 
# -labsl_raw_logging_internal 
# -labsl_log_severity 
# -lcurl 
# -lssl 
# -lcrypto

# # -L
# GCS_CXXLDFLAGS=`pkg-config $PKG_NAMES --libs-only-L`
# -L$WYLESLIBS_BUILD_ROOT_DIR/google-cloud-cpp/cmake-out/google/cloud/storage/../../lib 
# -L$WYLESLIBS_BUILD_ROOT_DIR/google-cloud-cpp/cmake-out/google/cloud/../../lib