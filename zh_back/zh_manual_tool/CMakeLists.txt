cmake_minimum_required(VERSION 3.10)
project(zh_manual_tool)

SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(. TEST_SRC)
aux_source_directory(../../zh_pub/zh_rpc_base/gen_code/cpp RPC_IMP_SRC)

add_executable(zh_manual_tool ${TEST_SRC} ${RPC_IMP_SRC})

target_link_libraries(zh_manual_tool thrift)
