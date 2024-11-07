#!/bin/bash

GCS_INSTALLED_FROM_VCPKG_REPO="NO"
while true; do
	case "$1" in
		--gcs_installed_from_vcpkg_repo) GCS_INSTALLED_FROM_VCPKG_REPO="YES"; shift 1 ;;
		*) break;;
	esac
done

if [ -z $WYLESLIBS_BUILD_ROOT_DIR ]; then
	WYLESLIBS_BUILD_ROOT_DIR="."
fi
# Google Cloud Storage
# ! IMPORTANT -
# 		Make sure to run `$WYLESLIBS_BUILD_ROOT_DIR/build_scripts/build_google.sh` before this script to make sure google deps are initialized.
# 		if installed using vcpkg then paths are appropriately defaulted to suit that install directory stucture.

# 	To use debug vcpkg libs set GOOGLE_CLOUD_VCPKG_LIB_DIR=google_cloud_cpp_vcpkg_install/x64-linux/debug/lib
#		TBD for cmake
if [ -z $GOOGLE_CLOUD_SRC_DIR ]; then
    GOOGLE_CLOUD_SRC_DIR="$WYLESLIBS_BUILD_ROOT_DIR/google-cloud-cpp"
fi

if [ -z $GOOGLE_CLOUD_BUILD_DIR ]; then
    GOOGLE_CLOUD_BUILD_DIR="$WYLESLIBS_BUILD_ROOT_DIR/google_cloud_cpp_cmake_build"
fi

if [ -z $GOOGLE_CLOUD_INSTALL_DIR ]; then
	if [ $GCS_INSTALLED_FROM_VCPKG_REPO == "NO" ]; then
		# assuming BUILD_FROM_SOURCE default location.
    	GOOGLE_CLOUD_INSTALL_DIR="$WYLESLIBS_BUILD_ROOT_DIR/google_cloud_cpp_cmake_install"
	else
    	GOOGLE_CLOUD_INSTALL_DIR="$WYLESLIBS_BUILD_ROOT_DIR/google_cloud_cpp_vcpkg_install"
	fi
fi
if [ -z $GOOGLE_CLOUD_VCPKG_INSTALL_DIR ]; then
	if [ $GCS_INSTALLED_FROM_VCPKG_REPO == "NO" ]; then
    	GOOGLE_CLOUD_VCPKG_INSTALL_DIR="$GOOGLE_CLOUD_BUILD_DIR/vcpkg_installed/x64-linux"
	else
    	GOOGLE_CLOUD_VCPKG_INSTALL_DIR="$GOOGLE_CLOUD_INSTALL_DIR/x64-linux"
	fi
fi
if [ -z $GOOGLE_CLOUD_LIB_DIR ]; then
	if [ $GCS_INSTALLED_FROM_VCPKG_REPO == "NO" ]; then
	    GOOGLE_CLOUD_LIB_DIR="$GOOGLE_CLOUD_INSTALL_DIR/lib"
	else 
		GOOGLE_CLOUD_LIB_DIR=""
	fi
fi
if [ -z $GOOGLE_CLOUD_VCPKG_LIB_DIR ]; then
    GOOGLE_CLOUD_VCPKG_LIB_DIR="$GOOGLE_CLOUD_VCPKG_INSTALL_DIR/lib"
fi

GCS_INCLUDE_DIRS="-I $GOOGLE_CLOUD_INSTALL_DIR/include -I $GOOGLE_CLOUD_VCPKG_INSTALL_DIR/include"
GCS_PKG_CONFIG_PATH="$GOOGLE_CLOUD_LIB_DIR/pkgconfig:$GOOGLE_CLOUD_VCPKG_LIB_DIR/pkgconfig"
PKG_NAMES="google_cloud_cpp_storage google_cloud_cpp_common google_cloud_cpp_rest_internal"
# if empty then won't build.
GCS_LIBS=`PKG_CONFIG_PATH=${GCS_PKG_CONFIG_PATH} pkg-config $PKG_NAMES --libs-only-l`

# ! IMPORTANT - g++ manual says shared libs are prioritized unless -static option is used
# TODO: libs with debug symbols?
if [ "$GOOGLE_CLOUD_LIB_DIR" != "" ]; then
	GCS_LIBS_SEARCH_PATH="$GCS_LIBS_SEARCH_PATH -L $GOOGLE_CLOUD_LIB_DIR"
fi
GCS_LIBS_SEARCH_PATH="$GCS_LIBS_SEARCH_PATH -L $GOOGLE_CLOUD_VCPKG_LIB_DIR"

echo -n "$GCS_INCLUDE_DIRS $GCS_LIBS_SEARCH_PATH $GCS_LIBS"
