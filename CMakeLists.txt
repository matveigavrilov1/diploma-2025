cmake_minimum_required(VERSION 3.10.0)
project(diploma VERSION 0.1.0 LANGUAGES C CXX)

set(CMAKE_CXX_STANDARD 20)
set(CMAKE_CXX_STANDARD_REQUIRED ON)

include(FetchContent)
FetchContent_Declare(MoodyCamel
    GIT_REPOSITORY https://github.com/cameron314/concurrentqueue
    GIT_TAG v1.0.4)
FetchContent_MakeAvailable(MoodyCamel)

add_subdirectory(src)