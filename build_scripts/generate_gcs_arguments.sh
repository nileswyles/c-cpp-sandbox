#!/bin/bash

if [ -z $WYLESLIBS_BUILD_ROOT_DIR ]; then
	WYLESLIBS_BUILD_ROOT_DIR="."
fi
# Google Cloud Storage
# ! IMPORTANT -
# 		Make sure to run `$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/build_google.sh` before this script to make sure google deps are initialized.
# 		if installed using vcpkg then paths are appropriately defaulted to suit that install directory stucture.

# 	To use debug vcpkg libs set GOOGLE_CLOUD_VCPKG_LIB_DIR=google_cloud_cpp_vcpkg_install/x64-linux/debug/lib
#		TBD for cmake

# assuming using vcpkg install command. If built from source, the build script will appropriately initialize the environment.
if [ -z $GOOGLE_CLOUD_VCPKG_INSTALL_DIR ]; then
    GOOGLE_CLOUD_VCPKG_INSTALL_DIR="$WYLESLIBS_BUILD_ROOT_DIR/google_cloud_cpp_install/x64-linux"
fi
if [ -z $GOOGLE_CLOUD_VCPKG_LIB_DIR ]; then
    GOOGLE_CLOUD_VCPKG_LIB_DIR="$GOOGLE_CLOUD_VCPKG_INSTALL_DIR/lib"
fi

GCS_INCLUDE_DIRS="-I $GOOGLE_CLOUD_VCPKG_INSTALL_DIR/include"
GCS_PKG_CONFIG_PATH="$GOOGLE_CLOUD_VCPKG_LIB_DIR/pkgconfig"
# PKG_NAMES="google_cloud_cpp_storage google_cloud_cpp_common google_cloud_cpp_rest_internal"
PKG_NAMES="google_cloud_cpp_storage"
# if empty then won't build.
GCS_LIBS=`PKG_CONFIG_PATH=${GCS_PKG_CONFIG_PATH} pkg-config $PKG_NAMES --libs-only-l`

# ! IMPORTANT - g++ manual says shared libs are prioritized unless -static option is used
# TODO: libs with debug symbols?
GCS_LIBS_SEARCH_PATH="$GCS_LIBS_SEARCH_PATH -L $GOOGLE_CLOUD_VCPKG_LIB_DIR"

echo -n "$GCS_INCLUDE_DIRS $GCS_LIBS_SEARCH_PATH $GCS_LIBS"
