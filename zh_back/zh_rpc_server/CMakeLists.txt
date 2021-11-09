cmake_minimum_required(VERSION 3.10)
project(zh_rpc_server)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
add_compile_options(-std=gnu++11 -g -rdynamic)


aux_source_directory(. RPC_SRC)
aux_source_directory(../../zh_pub/zh_rpc_base/gen_code/cpp RPC_IMP_SRC)

find_library(SQLITE_ORM_LIB libsqlite_orm.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(TDF_LIB libtcp_framework.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(RASTER_LIB libzh_raster.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(ID_READER libzh_id_reader.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(HK_GATE libzh_hk_gate.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(SCALE libzh_scale.so ${PRJ_INTERNAL_BUILD}/lib)

add_library(zh_rpc_server SHARED ${RPC_SRC} ${RPC_IMP_SRC})

target_link_libraries(zh_rpc_server pthread rt thrift uuid ${SQLITE_ORM_LIB} ${TDF_LIB} ${RASTER_LIB} ${ID_READER} ${HK_GATE} ${SCALE})