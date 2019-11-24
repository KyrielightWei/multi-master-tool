cmake_minimum_required (VERSION 3.10)

project(common_net_lib)

ADD_SUBDIRECTORY(extra/libevent)
include_directories(${CMAKE_SOURCE_DIR}/extra/libevent/include())
add_subdirectory(src)

## Test
enable_testing()
add_subdirectory(test)
