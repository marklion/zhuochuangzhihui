cmake_minimum_required(VERSION 3.10)
project(local_dev)

SET(LIBRARY_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/lib)
SET(EXECUTABLE_OUTPUT_PATH ${PRJ_INTERNAL_BUILD}/bin)
add_compile_options(-std=gnu++11 -g)
include_directories(${PROJECT_SOURCE_DIR})

aux_source_directory(./utils UTILS_SRC)
aux_source_directory(./process_manager PM_SRC)
aux_source_directory(./device_driver ABS_DD_SRC)
aux_source_directory(./device_driver/fake_cam FAKE_CAM_SRC)
aux_source_directory(./state_machine/scale_sm1 SCALE_SM1)

find_library(TDF_LIB libtcp_framework.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(VCOM_LIB libzh_vcom.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(SQLITE_ORM_LIB libsqlite_orm.so ${PRJ_INTERNAL_BUILD}/lib)

add_library(local_dev_util SHARED ${UTILS_SRC})
add_library(abs_device_driver SHARED ${ABS_DD_SRC})
add_executable(process_manager ${PM_SRC})
add_executable(fake_cam ${FAKE_CAM_SRC})
add_executable(scale_sm1 ${SCALE_SM1})

target_link_libraries(local_dev_util ${TDF_LIB})
target_link_libraries(abs_device_driver local_dev_util rt)
target_link_libraries(process_manager local_dev_util rt)
target_link_libraries(fake_cam abs_device_driver)
target_link_libraries(scale_sm1 local_dev_util rt)
