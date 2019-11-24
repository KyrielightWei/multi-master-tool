
aux_source_directory(. DIR_LIB_SRCS)

add_library(common_net_lib ${DIR_LIB_SRCS})

target_link_libraries(common_net_lib event)
target_link_libraries(common_net_lib pthread)