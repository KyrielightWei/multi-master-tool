cmake_minimum_required(VERSION 3.10)

project(multi_master_tool)

option(debug "cmake build mode" OFF)

SET(CMAKE_CXX_FLAGS_DEBUG "$ENV{CXXFLAGS} -O0 -Wall -g2 -ggdb")
SET(CMAKE_CXX_FLAGS_RELEASE "$ENV{CXXFLAGS} -O3 -Wall")

IF(debug)
    message(STATUS "DEBUG!!!!!")
    add_subdirectory(easy_logger)
    SET(CMAKE_BUILD_TYPE "Debug")
ENDIF(debug)

add_subdirectory(multi_net_io)

add_subdirectory(centralized-lock-table)