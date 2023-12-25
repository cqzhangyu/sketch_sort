cmake_minimum_required(VERSION 3.10)

project(sketch_sort)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
# set(default_build_type "Release")
# set(default_build_type "Debug")
set(CMAKE_BUILD_TYPE "Release")

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17 -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -march=native")

add_subdirectory(extern/tlx)
add_subdirectory(extern/CPPSFMT)

link_libraries(tlx dsfmt)

find_package(Threads REQUIRED)
link_libraries(Threads::Threads)

add_executable(cm_trie src/cm_trie.cpp)
add_executable(switch_sort src/switch_sort.cpp)
add_executable(sketch_test src/sketch_test.cpp)
