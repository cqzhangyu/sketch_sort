# @file CMakeLists.txt
#
# @author Michael Axtmann <michael.axtmann@gmail.com>
#
# The new BSD License is applied to this software, see LICENSE.txt

cmake_minimum_required(VERSION 3.10)

project(dSFMT)

set(CMAKE_MODULE_PATH ${CMAKE_CURRENT_SOURCE_DIR}/cmake/)
include(default_build_type)

set(CMAKE_CXX_FLAGS_RELEASE "${CMAKE_CXX_FLAGS_RELEASE} -std=c++17 -DNDEBUG -march=native")
set(CMAKE_CXX_FLAGS_DEBUG "${CMAKE_CXX_FLAGS_DEBUG} -std=c++17 -march=native")

add_library(dsfmt SHARED src/dSFMT.cpp src/SFMT.cpp)
target_include_directories(dsfmt PRIVATE extern/dSFMT)
target_include_directories(dsfmt PRIVATE extern/SFMT)
target_include_directories(dsfmt PUBLIC include)
target_compile_options(dsfmt PRIVATE  -msse2 -DHAVE_SSE2=1 -DDSFMT_MEXP=19937 -DSFMT_MEXP=19937)
