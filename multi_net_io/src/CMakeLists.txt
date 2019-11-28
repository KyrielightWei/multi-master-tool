
aux_source_directory(. DIR_LIB_SRCS)

add_library(multi_net_io ${DIR_LIB_SRCS})

target_link_libraries(multi_net_io event)
target_link_libraries(multi_net_io pthread)
