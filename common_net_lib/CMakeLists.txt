cmake_minimum_required (VERSION 3.10)

project(common_net_lib)

add_subdirectory(src)

## Test
enable_testing()
add_subdirectory(test)
