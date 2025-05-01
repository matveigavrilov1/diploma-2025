cmake_minimum_required(VERSION 3.10.0)
project(diploma VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

set(BUILD_TEST ON)


include(FetchContent)

FetchContent_Declare(spdlog
    GIT_REPOSITORY https://github.com/gabime/spdlog
    GIT_TAG v1.15.2) 
FetchContent_MakeAvailable(spdlog)

FetchContent_Declare(MoodyCamel
    GIT_REPOSITORY https://github.com/cameron314/concurrentqueue
    GIT_TAG v1.0.4)
FetchContent_MakeAvailable(MoodyCamel)

include_directories(src)
add_subdirectory(src)

if (${BUILD_TEST})
    FetchContent_Declare(googletest
    GIT_REPOSITORY https://github.com/google/googletest/
    GIT_TAG v1.16.0)
    FetchContent_MakeAvailable(googletest)

    enable_testing()
    include(GoogleTest)
    add_subdirectory(tests/unit)
endif()