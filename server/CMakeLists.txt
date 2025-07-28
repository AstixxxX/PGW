cmake_minimum_required(VERSION 3.14)
project(PGW_Server_Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/network
    ${CMAKE_SOURCE_DIR}/../utility
)

set(SERVER_SOURCES
    ${CMAKE_SOURCE_DIR}/server.cpp
    ${CMAKE_SOURCE_DIR}/network/udp_server.cpp
    ${CMAKE_SOURCE_DIR}/../utility/logger.h
    ${CMAKE_SOURCE_DIR}/../utility/json-config.h
    ${CMAKE_SOURCE_DIR}/../utility/timer.h
    ${CMAKE_SOURCE_DIR}/../utility/imsi_utils.h
    ${CMAKE_SOURCE_DIR}/../utility/httplib.h
)

add_executable(pgw_server ${SERVER_SOURCES})

target_link_libraries(pgw_server 
    PRIVATE 
    nlohmann_json::nlohmann_json
    pthread
)

configure_file(
    config_pgw.json
    ${CMAKE_BINARY_DIR}/config_pgw.json
    COPYONLY
)

configure_file(
    blacklist.txt
    ${CMAKE_BINARY_DIR}/blacklist.txt
    COPYONLY
)

configure_file(
    pgw.log
    ${CMAKE_BINARY_DIR}/pgw.log
    COPYONLY
)

configure_file(
    cdr.log
    ${CMAKE_BINARY_DIR}/cdr.log
    COPYONLY
)