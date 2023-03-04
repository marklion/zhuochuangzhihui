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
aux_source_directory(./device_driver/zs_cam ZS_CAM_SRC)
aux_source_directory(./device_driver/kl_scale KL_SCALE_SRC)
aux_source_directory(./device_driver/ser_id_reader SER_ID_READER)
aux_source_directory(./device_driver/ser_qr_reader SER_QR_READER)
aux_source_directory(./device_driver/ser_printer SER_PRINTER)
aux_source_directory(./state_machine/scale_sm1 SCALE_SM1)
aux_source_directory(../../zh_pub/zh_cpp_pub PUB_SRC)


add_library(local_dev_util SHARED ${UTILS_SRC} ${PUB_SRC})
add_library(abs_device_driver SHARED ${ABS_DD_SRC})
add_executable(process_manager ${PM_SRC})
add_executable(fake_cam ${FAKE_CAM_SRC})
add_executable(zs_cam ${ZS_CAM_SRC})
add_executable(kl_scale ${KL_SCALE_SRC})
add_executable(ser_id_reader ${SER_ID_READER})
add_executable(ser_qr_reader ${SER_QR_READER})
add_executable(ser_printer ${SER_PRINTER})
add_executable(scale_sm1 ${SCALE_SM1})

add_custom_command(TARGET zs_cam
                   PRE_BUILD
                   COMMAND cp ${PROJECT_SOURCE_DIR}/device_driver/zs_cam/libVzLPRSDK.so ${PRJ_INTERNAL_BUILD}/lib/)

find_library(ZS_CAM_SDK_LIB libVzLPRSDK.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(TDF_LIB libtcp_framework.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(VCOM_LIB libzh_vcom.so ${PRJ_INTERNAL_BUILD}/lib)
find_library(SQLITE_ORM_LIB libsqlite_orm.so ${PRJ_INTERNAL_BUILD}/lib)

target_link_libraries(local_dev_util ${TDF_LIB})
target_link_libraries(abs_device_driver local_dev_util rt)
target_link_libraries(process_manager local_dev_util rt)
target_link_libraries(fake_cam abs_device_driver)
target_link_libraries(zs_cam abs_device_driver ${ZS_CAM_SDK_LIB} pthread)
target_link_libraries(kl_scale abs_device_driver)
target_link_libraries(ser_id_reader abs_device_driver ${VCOM_LIB} termb)
target_link_libraries(ser_qr_reader abs_device_driver)
target_link_libraries(ser_printer abs_device_driver)
target_link_libraries(scale_sm1 ${SQLITE_ORM_LIB} local_dev_util rt)

