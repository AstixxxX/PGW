cmake_minimum_required(VERSION 3.14)
project(UDP_Client_Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/utility
    ${CMAKE_SOURCE_DIR}/network
)

set(SOURCES
    ${CMAKE_SOURCE_DIR}/client.cpp
    ../utility/logger.h
    ../utility/json-config.h
    ../utility/timer.h
    ../utility/imsi_utils.h
    ./network/udp_client.cpp
)

add_executable(pgw_client ${SOURCES})

target_link_libraries(pgw_client PRIVATE pthread)