cmake_minimum_required(VERSION 3.10)
project(zh_id_reader)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(./lib API_SRC)
aux_source_directory(. TEST_SRC)

find_library(VCOM_LIB libzh_vcom.so ${PRJ_INTERNAL_BUILD}/lib)

add_library(zh_id_reader SHARED ${API_SRC})
add_executable(zh_id_reader_test ${TEST_SRC})

target_link_libraries(zh_id_reader_test zh_id_reader)
target_link_libraries(zh_id_reader termb ${VCOM_LIB})
