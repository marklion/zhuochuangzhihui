cmake_minimum_required(VERSION 3.10)
project(zh_raster)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(./lib API_SRC)
aux_source_directory(. TEST_SRC)

find_library(VCOM_LIB libzh_vcom.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(TDF_LIB libtcp_framework.so ${PRJ_INTERNAL_BUILD}/lib)

add_library(zh_raster SHARED ${API_SRC})
add_executable(zh_raster_test ${TEST_SRC})

target_link_libraries(zh_raster_test zh_raster)
target_link_libraries(zh_raster termb ${VCOM_LIB} modbus ${TDF_LIB})
