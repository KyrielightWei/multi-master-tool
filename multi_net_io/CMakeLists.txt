cmake_minimum_required (VERSION 3.10)
set(CMAKE_CXX_STANDARD 11)

project(multi_net_io)

ADD_SUBDIRECTORY(extra/libevent)
include_directories(${CMAKE_SOURCE_DIR}/extra/libevent/include)
include_directories(${CMAKE_SOURCE_DIR}/include)
add_subdirectory(src)

## Test
enable_testing()
add_subdirectory(test)
