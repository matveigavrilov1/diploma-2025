cmake_minimum_required(VERSION 3.10.0)

set(BENCHMARK_TARGET_NAME coroMutexBenchmark)

add_executable(${BENCHMARK_TARGET_NAME} main.cpp ${CORE_SOURCES})

target_link_libraries(${BENCHMARK_TARGET_NAME}
    PRIVATE 
        concurrentqueue)