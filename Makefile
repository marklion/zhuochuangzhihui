SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build
SUB_DIR=zh_pub zh_back zh_external zh_front

all:$(DELIVER_PATH)
	tar zcf zczh_software_$(shell date +%Y%m%d%H%M%S).tar.gz -C $(SRC_DIR) build

$(DELIVER_PATH):$(SUB_DIR)
	[ -d $@ ] || mkdir $@
	for component in $^;do [ -d $$component/build ] && cp -a $$component/build/* $@/; done

$(SUB_DIR):
	make -C $(SRC_DIR)/$@

zh_back:zh_pub zh_external

zh_front:zh_pub zh_external

clean:
	rm -rf $(DELIVER_PATH)
	for sub_component in $(SUB_DIR); do make clean -C $(SRC_DIR)/$$sub_component;done

.PHONY:all $(SUB_DIR) $(DELIVER_PATH) clean