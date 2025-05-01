cmake_minimum_required(VERSION 3.10.0)

set(CORE_SOURCES
    ${CMAKE_SOURCE_DIR}/src/core/task.cpp
    ${CMAKE_SOURCE_DIR}/src/core/thread-pool.cpp
    ${CMAKE_SOURCE_DIR}/src/core/task-manager.cpp
    ${CMAKE_SOURCE_DIR}/src/core/coro-mutex.cpp
)

set(RACE_CONDITION_TARGET_NAME race_condition)
set(CORO_MUTEX_TEST_TARGET_NAME coro_mutex_test)
set(STD_MUTEX_TEST_TARGET_NAME std_mutex_test)

add_executable(${RACE_CONDITION_TARGET_NAME} race-condition/main.cpp ${CORE_SOURCES})

target_link_libraries(${RACE_CONDITION_TARGET_NAME}
    PRIVATE 
        concurrentqueue)

add_executable(${CORO_MUTEX_TEST_TARGET_NAME} coro-mutex-test/main.cpp ${CORE_SOURCES})

target_link_libraries(${CORO_MUTEX_TEST_TARGET_NAME}
    PRIVATE 
        concurrentqueue)

add_executable(${STD_MUTEX_TEST_TARGET_NAME} std-mutex-test/main.cpp ${CORE_SOURCES})

target_link_libraries(${STD_MUTEX_TEST_TARGET_NAME}
    PRIVATE 
        concurrentqueue)

add_subdirectory(benchmark)