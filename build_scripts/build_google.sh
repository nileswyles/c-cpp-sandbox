git -C $HOME clone https://github.com/microsoft/vcpkg.git
env VCPKG_ROOT=$HOME/vcpkg $HOME/vcpkg/bootstrap-vcpkg.sh
cmake -S google-cloud-cpp -B google-cloud-cpp/cmake-out/ -DCMAKE_TOOLCHAIN_FILE=$HOME/vcpkg/scripts/buildsystems/vcpkg.cmake
#cmake --build google-cloud-cpp/cmake-out -- -j $(nproc)
cmake --build google-cloud-cpp/cmake-out -- -j 1