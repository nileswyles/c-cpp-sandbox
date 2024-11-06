#!/bin/bash

git -C $HOME clone https://github.com/microsoft/vcpkg.git
env VCPKG_ROOT=$HOME/vcpkg $HOME/vcpkg/bootstrap-vcpkg.sh

# usage (defaults for env variables):
#   ./build_google.sh 
#   ./build_google.sh BUILD_FROM_SOURCE
#   ./build_google.sh BUILD_FROM_SOURCE AND_INSTALL
#   ./build_google.sh BUILD_FROM_SOURCE AND_INSTALL
if [ -z $WYLESLIBS_BUILD_ROOT_DIR ]; then
	WYLESLIBS_BUILD_ROOT_DIR="."
fi
if [ -z $GOOGLE_CLOUD_SRC_DIR ]; then
    GOOGLE_CLOUD_SRC_DIR="$WYLESLIBS_BUILD_ROOT_DIR/google-cloud-cpp"
fi
if [ -z $GOOGLE_CLOUD_BUILD_DIR ]; then
    GOOGLE_CLOUD_BUILD_DIR="$WYLESLIBS_BUILD_ROOT_DIR/$GOOGLE_CLOUD_SRC_DIR/cmake-out"
fi
if [ -z $GOOGLE_CLOUD_INSTALL_DIR ]; then
    GOOGLE_CLOUD_BUILD_DIR="$WYLESLIBS_BUILD_ROOT_DIR/$GOOGLE_CLOUD_SRC_DIR/install"
fi
if [ "$1" == "CLEAN" ]; then
    rm -rf $GOOGLE_CLOUD_BUILD_DIR $GOOGLE_CLOUD_INSTALL_DIR
elif [ "$1" == "BUILD_FROM_SOURCE" ]; then
    # ! IMPORTANT
    # need to make sure we link to correct ssl, crypto. Make sure to check where google get's theirs from if it does an installation.

    # These are apparently not used by the project.
    # -DWITH_EXAMPLES=OFF \
    # -DWITH_ABSEIL=OFF \
    # -DOPENTELEMETRY_INSTALL=OFF \

    # default install appears to be /usr/local
    cmake -DCMAKE_BUILD_TYPE=Release \
        -DCMAKE_INSTALL_PREFIX="$GOOGLE_CLOUD_INSTALL_DIR" \
        -DBUILD_SHARED_LIBS=yes \
        -DBUILD_TESTING=OFF \
        -DGOOGLE_CLOUD_CPP_ENABLE=storage \
        -S $GOOGLE_CLOUD_SRC_DIR -B $GOOGLE_CLOUD_BUILD_DIR -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
    cmake --build $GOOGLE_CLOUD_BUILD_DIR -- -j $(nproc)
    # if this installs to same directory as vcpkg then can simplify http_server.sh
    if [ "$2" == "AND_INSTALL" ]; then
        # default install location appears to be /usr/local
        # TODO: I AM NOT SURE WHAT'S GAINED FROM INSTALLING 
        # sudo cmake --build $GOOGLE_CLOUD_BUILD_DIR --target install
        echo "NOT SURE WHAT'S GAINED FROM INSTALLING SO DO NOTHING HERE FOR NOW"
    fi
else
    $HOME/vcpkg/vcpkg install google-cloud-cpp --x-install-root=$GOOGLE_CLOUD_INSTALL_DIR
    # otherwise:
    # /home/vscode/vcpkg/packages/google-cloud-cpp_x64-linux/lib/pkgconfig
    # /home/vscode/vcpkg/packages/google-cloud-cpp_x64-linux/lib/*.a
    # /home/vscode/vcpkg/packages/google-cloud-cpp_x64-linux/include
fi
