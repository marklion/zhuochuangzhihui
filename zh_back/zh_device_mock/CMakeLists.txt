cmake_minimum_required(VERSION 3.10)
project(zh_device_mock)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(. TEST_SRC)

find_library(TDF_LIB libtcp_framework.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(DATABASE_LIB libsqlite_orm.so ${PRJ_INTERNAL_BUILD}/lib)

add_executable(zh_device_mock ${TEST_SRC})

target_link_libraries(zh_device_mock ${TDF_LIB} ${DATABASE_LIB})
