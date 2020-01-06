CMAKE_MINIMUM_REQUIRED(VERSION 3.10)

PROJECT(centralized-lock-table)

#ADD_SUBDIRECTORY(../multi_net_io)

AUX_SOURCE_DIRECTORY(server LOCK_SERVER_SOURCE)

SET(MESS_CONFIG_DIR "${CMAKE_SOURCE_DIR}/centralized-lock-table")

CONFIGURE_FILE("server/server_config.h.in" "server/server_config.h")

INCLUDE_DIRECTORIES(include)

IF(debug)
INCLUDE_DIRECTORIES("../multi_net_io/include")
INCLUDE_DIRECTORIES("../multi_net_io/libevent/include")
INCLUDE_DIRECTORIES(${CMAKE_BINARY_DIR}/multi_net_io/libevent/include)
ENDIF()

ADD_EXECUTABLE(centralized-lock-server ${LOCK_SERVER_SOURCE})
TARGET_LINK_LIBRARIES(centralized-lock-server multi_net_io)