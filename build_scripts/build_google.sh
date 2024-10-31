git -C $HOME clone https://github.com/microsoft/vcpkg.git
env VCPKG_ROOT=$HOME/vcpkg $HOME/vcpkg/bootstrap-vcpkg.sh

# from packaging.md, abseil install...
#   I instead used libabsl-dev from ubuntu. It should be off anyways - just need the header files for compiling?

# ```bash
# mkdir -p $HOME/Downloads/abseil-cpp && cd $HOME/Downloads/abseil-cpp
# curl -fsSL https://github.com/abseil/abseil-cpp/archive/20240722.0.tar.gz | \
#     tar -xzf - --strip-components=1 && \
#     cmake \
#       -DCMAKE_BUILD_TYPE=Release \
#       -DABSL_BUILD_TESTING=OFF \
#       -DABSL_PROPAGATE_CXX_STD=ON \
#       -DBUILD_SHARED_LIBS=yes \
#       -S . -B cmake-out && \
#     cmake --build cmake-out -- -j ${NCPU:-4} && \
# sudo cmake --build cmake-out --target install -- -j ${NCPU:-4} && \
# sudo ldconfig
# ```

cmake -DCMAKE_BUILD_TYPE=Release \
    -DBUILD_SHARED_LIBS=no \
    -DWITH_EXAMPLES=OFF \
    -DWITH_ABSEIL=OFF \
    -DBUILD_TESTING=OFF \
    -DOPENTELEMETRY_INSTALL=OFF \
    -DGOOGLE_CLOUD_CPP_ENABLE=storage \
    -S google-cloud-cpp -B google-cloud-cpp/cmake-out/ -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
cmake --build google-cloud-cpp/cmake-out -- -j $(nproc)
# cmake --build google-cloud-cpp/cmake-out -- -j 1