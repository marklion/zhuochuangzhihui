cmake_minimum_required(VERSION 3.10)
project(tcp_framework)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(. FRAMEWORK_SRC)
aux_source_directory(./test TIMER_TEST_SRC)
add_library(tcp_framework SHARED ${FRAMEWORK_SRC})
target_link_libraries(tcp_framework pthread rt)

add_executable(timer_test ${TIMER_TEST_SRC})
target_link_libraries(timer_test tcp_framework)