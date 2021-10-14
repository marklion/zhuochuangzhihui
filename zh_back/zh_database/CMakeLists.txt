cmake_minimum_required(VERSION 3.10)
project(zh_database)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(. API_SRC)
add_library(sqlite_orm SHARED ${API_SRC})

find_library(TDF_LIB libtcp_framework.so ${PRJ_INTERNAL_BUILD}/lib)

target_link_libraries(sqlite_orm sqlite3 ${TDF_LIB})
