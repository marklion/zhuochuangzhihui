SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build
SUB_DIR=zh_rpc_server zh_database zh_tdf zh_vcom zh_id_reader zh_raster zh_device_mock zh_hk_gate zh_rest_server zh_scale zh_manual_tool zh_qr

all:$(DELIVER_PATH)/bin/zh_daemon
	cp -a $(SRC_DIR)/zh_rest_server/build/zh_rest_node $(DELIVER_PATH)/

$(DELIVER_PATH):
	[ -d $(DELIVER_PATH) ] || mkdir $(DELIVER_PATH)
	[ -d $(DELIVER_PATH)/bin ] || mkdir $(DELIVER_PATH)/bin
	[ -d $(DELIVER_PATH)/lib ] || mkdir $(DELIVER_PATH)/lib

$(DELIVER_PATH)/bin/zh_daemon:$(SUB_DIR) main.cpp $(DELIVER_PATH)
	g++ -std=gnu++11 -g -o $@ main.cpp -lpthread -lthrift -lssl -lcrypto -L $(DELIVER_PATH)/lib -l zh_rpc_server -lsqlite_orm -lzh_id_reader -lzh_hk_gate -Wl,-rpath=../lib -ltcp_framework

zh_rpc_server:$(DELIVER_PATH) zh_database zh_tdf zh_vcom zh_id_reader zh_raster zh_hk_gate zh_scale zh_qr
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_database:$(DELIVER_PATH) zh_tdf
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_tdf:$(DELIVER_PATH)
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_vcom:$(DELIVER_PATH) zh_tdf
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_id_reader:$(DELIVER_PATH) zh_vcom
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_raster:$(DELIVER_PATH) zh_vcom
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_scale:$(DELIVER_PATH) zh_vcom
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_device_mock:$(DELIVER_PATH) zh_tdf zh_database
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_hk_gate:$(DELIVER_PATH) zh_tdf
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_qr:$(DELIVER_PATH) zh_tdf
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

zh_rest_server: $(DELIVER_PATH)
	$(MAKE) -C $(SRC_DIR)/$@

zh_manual_tool:$(DELIVER_PATH)
	[ -d $(SRC_DIR)/$(@)/build ] || mkdir $(SRC_DIR)/$@/build
	pushd $(SRC_DIR)/$@/build;[ -f Makefile ] || cmake -D PRJ_INTERNAL_BUILD=$(DELIVER_PATH) ../; $(MAKE)

clean:
	rm -rf $(DELIVER_PATH)
	for sub_component in $(SUB_DIR); do rm -rf $(SRC_DIR)/$$sub_component/build;done

.PHONY:all $(SUB_DIR) clean