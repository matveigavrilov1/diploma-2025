cmake_minimum_required(VERSION 3.10.0)

project(UnitTests LANGUAGES CXX)

file(GLOB_RECURSE SOURCE "${CMAKE_SOURCE_DIR}/src/core/*.cpp")
file(GLOB_RECURSE TEST_SOURCE "${CMAKE_SOURCE_DIR}/tests/unit/*.cpp")

add_executable(${PROJECT_NAME} ${TEST_SOURCE} ${SOURCE})

gtest_discover_tests(${PROJECT_NAME})

target_link_libraries(${PROJECT_NAME}
    PRIVATE
        concurrentqueue
        GTest::gtest_main)