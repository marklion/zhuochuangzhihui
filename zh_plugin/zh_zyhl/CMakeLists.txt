cmake_minimum_required(VERSION 3.10)
project(zh_zyhl)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(./lib API_SRC)
aux_source_directory(./ PLUGIN_SRC)
aux_source_directory(../../zh_pub/zh_cpp_pub PUB_SRC)
aux_source_directory(../../zh_pub/zh_rpc_base/gen_code/cpp GEN_SRC)
aux_source_directory(../../zh_back/zh_tdf TDF_SRC)

add_library(zh_zyhl STATIC ${API_SRC} ${PUB_SRC} ${TDF_SRC} ${GEN_SRC})
add_executable(zh_zyhl_plugin ${PLUGIN_SRC})

target_link_libraries(zh_zyhl_plugin zh_zyhl pthread rt modbus thrift ssl crypto)
