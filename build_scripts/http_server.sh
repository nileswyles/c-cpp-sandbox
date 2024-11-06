#!/bin/sh

TEST_ARG=""
DEFINES=""
LOG_LEVEL=0
GCS_LINK_TO_SOURCES="NO"
while true; do
	case "$1" in
		-l|--log) LOG_LEVEL="$2"; shift 2 ;;
		-D) DEFINES="$DEFINES-D $2 "; shift 2 ;;
		--gcs_from_source) GCS_LINK_TO_SOURCES="YES"; shift 2 ;;
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
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/estream.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/estream/reader_task.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/web/server.c
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file_watcher.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/web/http/http_file_watcher.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/datastructures/array.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/stream_factory.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/file_gcs.cpp
-s $WYLESLIBS_BUILD_ROOT_DIR/lib/file/stream_factory_gcs.cpp
"

INCLUDE_DIRS="-I $WYLESLIBS_BUILD_ROOT_DIR/http_test"

LD_FLAGS="
-l ssl
-l crypto
"

# Google Cloud Storage
# ! IMPORTANT -
# 		Make sure to run `$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/build_google.sh` before this script to make sure google deps are initialized.
if [ -z $GOOGLE_CLOUD_SRC_DIR ]; then
    GOOGLE_CLOUD_SRC_DIR="$WYLESLIBS_BUILD_ROOT_DIR/google-cloud-cpp"
fi
if [ -z $GOOGLE_CLOUD_BUILD_DIR ]; then
    GOOGLE_CLOUD_BUILD_DIR="$WYLESLIBS_BUILD_ROOT_DIR/$GOOGLE_CLOUD_SRC_DIR/cmake-out"
fi
if [ -z $GOOGLE_CLOUD_INSTALL_DIR ]; then
    GOOGLE_CLOUD_INSTALL_DIR="$WYLESLIBS_BUILD_ROOT_DIR/$GOOGLE_CLOUD_SRC_DIR/install"
fi
GCS_INCLUDE_DIRS=""
GCS_LIB_SEARCH_PATH=""
GCS_LIBS=""
if [ $GCS_LINK_TO_SOURCES eq "YES" ]; then
	GOOGLE_CLOUD_OUT="$GOOGLE_CLOUD_BUILD_DIR/google/cloud"
	export PKG_CONFIG_PATH="$GOOGLE_CLOUD_OUT:$GOOGLE_CLOUD_OUT/storage"
	PKG_NAMES="google_cloud_cpp_storage google_cloud_cpp_common google_cloud_cpp_rest_internal"
	# CFLAGS_CMD="pkg-config $PKG_NAMES --cflags"
	# echo $CFLAGS_CMD
	# GCS_CXXFLAGS=`$CFLAGS_CMD`
	# GCS_CXXLDFLAGS=`pkg-config $PKG_NAMES --libs-only-L`
	GCS_INCLUDE_DIRS="-I $GOOGLE_CLOUD_SRC_DIR "
	GCS_LIB_SEARCH_PATH="-L $GOOGLE_CLOUD_OUT -L $GOOGLE_CLOUD_OUT/storage -L $GOOGLE_CLOUD_BUILD_DIR/vcpkg_installed/x64-linux/lib "
	GCS_LIBS=`pkg-config $PKG_NAMES --libs-only-l`

	# google-cloud-cpp
	# 	from pkg-config after building "and installing" using build_google.sh

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
else
# else built and installed using the vcpkg package `vcpkg install google-cloud-cpp` or with `cmake ... --target install`
	PKG_NAMES="google_cloud_cpp_storage google_cloud_cpp_common google_cloud_cpp_rest_internal"
	GCS_LIBS=`pkg-config $PKG_NAMES --libs-only-l`
fi
GCS_LIBS="$GCS_LIBS "

CMD="$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/build_common.sh -n http_server $SRC_FILES --log $LOG_LEVEL $INCLUDE_DIRS $LD_FLAGS $GCS_INCLUDE_DIRS$GCS_LIBS_SEARCH_PATH$GCS_LIBS$DEFINES$PROGRAM_ARG"
echo "\t"$CMD
exec $CMD