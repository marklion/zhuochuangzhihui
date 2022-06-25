cmake_minimum_required(VERSION 3.10)
project(zh_hk_gate)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g -L/lib/HCNetSDKCom)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(./lib API_SRC)
aux_source_directory(. TEST_SRC)

find_library(TDF_LIB libtcp_framework.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(SQLITE_ORM_LIB libsqlite_orm.so ${PRJ_INTERNAL_BUILD}/lib)

add_library(zh_hk_gate SHARED ${API_SRC})
add_executable(zh_hk_gate_test ${TEST_SRC})

target_link_libraries(zh_hk_gate HttpUtil ${TDF_LIB} hcnetsdk ${SQLITE_ORM_LIB})
target_link_libraries(zh_hk_gate_test zh_hk_gate ${TDF_LIB} ${SQLITE_ORM_LIB})
