# Copied from tutorial step 2

cmake_minimum_required(VERSION 3.10)

# set the project name and version
project(Test VERSION 1.0)

# specify the C++ standard
set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED True)

# configure a header file to pass some of the CMake settings
# to the source code
# configure_file(TutorialConfig.h.in TutorialConfig.h)


# add the executable
add_executable(Test main.cpp "${PROJECT_SOURCE_DIR}/../lib/datastructures/array.cpp")

# alternatively,
# Use add_subdirectory() to add Array source files to this project. CMakeList.txt under that directory should add relevant source files.
# Hint: ${PROJECT_SOURCE_DIR} is a path to the project source. AKA This folder!
# add_subdirectory("${PROJECT_SOURCE_DIR}/../lib/datastructures" "${PROJECT_SOURCE_DIR}/out/datastructures")

# TODO 3: Use target_link_libraries to link the library to our executable
# TODO 4: Add MathFunctions to Tutorial's target_include_directories()

# add the binary tree to the search path for include files
target_include_directories(Test PUBLIC
                           "${PROJECT_SOURCE_DIR}"
                           )
target_include_directories(Test PUBLIC
                           "${PROJECT_SOURCE_DIR}/../lib"
                           )
