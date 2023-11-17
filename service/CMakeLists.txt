cmake_minimum_required(VERSION 3.10)
project(core_service)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++14 -g)

aux_source_directory(./lib LIB_SRC)
aux_source_directory(./core CORE_SERVICE_SRC)
aux_source_directory(./device DEVICE_SERVICE_SRC)
aux_source_directory(./cli CLI_SRC)

add_library(rpc_lib SHARED ${LIB_SRC})
add_executable(core_service ${CORE_SERVICE_SRC})
add_executable(device_center ${DEVICE_SERVICE_SRC})
add_executable(zh_cli ${CLI_SRC})

add_executable(mock_driver ./device/drivers/mock_driver.cpp)
add_executable(zs_plate_cam ./device/drivers/zs_plate_cam.cpp)

find_library(IDL_LIB libzh_idl.so ${PRJ_INTERNAL_BUILD}/lib ../base/build/lib)
find_library(DB_LIB libsqlite_orm.so ${PRJ_INTERNAL_BUILD}/lib ../base/build/lib)
find_library(UTIL_LIB libzh_utils.so ${PRJ_INTERNAL_BUILD}/lib ../base/build/lib)
find_library(FILE_UP_DOWN_LIB libfile_up_down.so ${PRJ_INTERNAL_BUILD}/lib ../base/build/lib)

target_link_libraries(rpc_lib ${IDL_LIB} ${DB_LIB} ${UTIL_LIB} ${FILE_UP_DOWN_LIB})
target_link_libraries(core_service pthread thrift ${IDL_LIB} ${DB_LIB} ${UTIL_LIB} rpc_lib)
target_link_libraries(device_center pthread thrift ${IDL_LIB} ${DB_LIB} ${UTIL_LIB} rpc_lib)
target_link_libraries(zh_cli pthread thrift ${IDL_LIB} ${DB_LIB} ${UTIL_LIB} rpc_lib)

target_link_libraries(mock_driver pthread thrift ${IDL_LIB} ${DB_LIB} ${UTIL_LIB} rpc_lib)
target_link_libraries(zs_plate_cam pthread thrift ${IDL_LIB} ${DB_LIB} ${UTIL_LIB} rpc_lib)
