cmake_minimum_required(VERSION 3.14)
project(IMSI_Client_Tests)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

include(FetchContent)

FetchContent_Declare(
    googletest
    GIT_REPOSITORY https://github.com/google/googletest.git
    GIT_TAG v1.17.0
)

FetchContent_MakeAvailable(googletest)


add_executable(imsi_tests
    imsi_util_tests.cpp
)

target_link_libraries(imsi_tests
    PRIVATE
    GTest::gtest_main
    pthread  
)

enable_testing()
add_test(NAME IMSI_Tests COMMAND imsi_tests)