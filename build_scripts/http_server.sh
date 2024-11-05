#!/bin/sh

TEST_ARG=""
DEFINES=""
LOG_LEVEL=0
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES-D $2 "; shift 2 ;;
		@(-D*)) DEFINES="$DEFINES$2 "; shift 2 ;;
		*) PROGRAM_ARG=$@; break;;
	esac
done

DEFINES="$DEFINES-D WYLESLIBS_SSL_ENABLED=1 "

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
-s $ROOT_DIR/lib/estream/estream.cpp
-s $ROOT_DIR/lib/estream/reader_task.cpp
-s $ROOT_DIR/lib/web/server.c
-s $ROOT_DIR/lib/file/file_watcher.cpp
-s $ROOT_DIR/lib/web/http/http_file_watcher.cpp
-s $ROOT_DIR/lib/datastructures/array.cpp
-s $ROOT_DIR/lib/file/file.cpp
-s $ROOT_DIR/lib/file/stream_factory.cpp
-s $ROOT_DIR/lib/file/file_gcs.cpp
-s $ROOT_DIR/lib/file/stream_factory_gcs.cpp
"

# this should work just fine until we have larger projects and require caching individual objects?
GOOGLE_CLOUD_OUT="google-cloud-cpp/cmake-out/google/cloud"

PC_FILES="
$GOOGLE_CLOUD_OUT/storage/google_cloud_cpp_storage.pc
$GOOGLE_CLOUD_OUT/google_cloud_cpp_common.pc
$GOOGLE_CLOUD_OUT/google_cloud_cpp_rest_internals.pc
"

# TODO:
# need to make sure we link to the correct ssl, crypto for our stuff. reason enough to just use Makefiles now? 
# 	Or just trust google's ssl? maybe okay for non-production?
GCS_CXXFLAGS=`pkg-config $PC_FILES --cflags`
GCS_CXXLDFLAGS=`pkg-config $PC_FILES --libs-only-L`
GCS_LIBS=`pkg-config $PC_FILES --libs-only-l`

# google-cloud-cpp
# 	from pkg-config after building "and installing" using build_google.sh

# # cflags
# -DNOMINMAX 
# -I$ROOT_DIR/google-cloud-cpp/cmake-out/google/cloud/storage/../../include 
# -I$ROOT_DIR/google-cloud-cpp/cmake-out/google/cloud/../../include 
# -I/usr/include/x86_64-linux-gnu

# # -l
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
# -L$ROOT_DIR/google-cloud-cpp/cmake-out/google/cloud/storage/../../lib 
# -L$ROOT_DIR/google-cloud-cpp/cmake-out/google/cloud/../../lib

LD_FLAGS="
-l ssl
-l crypto
"

# $ROOT_DIR/build_scripts/build_google.sh

CMD="$ROOT_DIR/build_scripts/build_common.sh -n http_server $SRC_FILES --log $LOG_LEVEL $LD_FLAGS $GCS_CXXFLAGS $GCS_CXXLDFLAGS $GCS_LIBS $DEFINES$PROGRAM_ARG"
echo "\t"$CMD
exec $CMD