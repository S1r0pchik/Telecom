cmake_minimum_required(VERSION 3.14)
project(Telecom)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

add_executable(Telecom main.cpp processor.cpp)

include(FetchContent)
FetchContent_Declare(
        googletest
        URL https://github.com/google/googletest/archive/refs/tags/v1.14.0.zip
        DOWNLOAD_EXTRACT_TIMESTAMP true
)

set(gtest_force_shared_crt ON CACHE BOOL "" FORCE)
FetchContent_MakeAvailable(googletest)

enable_testing()

add_executable(processor_test processor_test.cpp processor.cpp)

target_link_libraries(processor_test gtest_main)

include(GoogleTest)
gtest_discover_tests(processor_test)