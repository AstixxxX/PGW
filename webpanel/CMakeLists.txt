cmake_minimum_required(VERSION 3.14)
project(PGW_Server_Project)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

find_package(nlohmann_json REQUIRED)

include_directories(
    ${CMAKE_SOURCE_DIR}
    ${CMAKE_SOURCE_DIR}/../utility
)

set(SERVER_SOURCES
    ${CMAKE_SOURCE_DIR}/webpanel.cpp
    ${CMAKE_SOURCE_DIR}/../utility/imsi_utils.h
    ${CMAKE_SOURCE_DIR}/../utility/httplib.h
)

add_executable(web ${SERVER_SOURCES})

target_link_libraries(web
    PRIVATE 
    nlohmann_json::nlohmann_json
    pthread
)