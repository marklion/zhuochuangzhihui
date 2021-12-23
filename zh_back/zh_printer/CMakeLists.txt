cmake_minimum_required(VERSION 3.10)
project(zh_printer)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(./lib API_SRC)
aux_source_directory(. TEST_SRC)

find_library(TDF_LIB libtcp_framework.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(VCOM_LIB libzh_vcom.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(SQLITE_ORM_LIB libsqlite_orm.so ${PRJ_INTERNAL_BUILD}/lib)

add_library(zh_printer SHARED ${API_SRC})
add_executable(zh_printer_test ${TEST_SRC})

target_link_libraries(zh_printer_test zh_printer)
target_link_libraries(zh_printer termb ${VCOM_LIB} ${TDF_LIB} ${SQLITE_ORM_LIB})
