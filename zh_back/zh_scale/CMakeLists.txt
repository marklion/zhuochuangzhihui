cmake_minimum_required(VERSION 3.10)
project(zh_scale)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(./lib API_SRC)
aux_source_directory(. TEST_SRC)

find_library(VCOM_LIB libzh_vcom.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(TDF_LIB libtcp_framework.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(SQLITE_ORM_LIB libsqlite_orm.so ${PRJ_INTERNAL_BUILD}/lib)

add_library(zh_scale SHARED ${API_SRC})
add_executable(zh_scale_test ${TEST_SRC})

target_link_libraries(zh_scale_test zh_scale)
target_link_libraries(zh_scale termb ${VCOM_LIB} modbus ${TDF_LIB} ${SQLITE_ORM_LIB})
