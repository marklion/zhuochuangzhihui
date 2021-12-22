SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build
SUB_DIR=zh_pub zh_back zh_external zh_front
BUILD_MODE=build
export BUILD_MODE

pack:all
	tar zcf zh_deliver.tar.gz -C $(DELIVER_PATH) bin lib conf manage_dist zh_rest_node script
	cat deploy.sh zh_deliver.tar.gz > $(DELIVER_PATH)/install.sh
	chmod +x $(DELIVER_PATH)/install.sh
	rm zh_deliver.tar.gz

all:$(DELIVER_PATH)

$(DELIVER_PATH):$(SUB_DIR)
	[ -d $@ ] || mkdir $@
	for component in $^;do [ -d $$component/build ] && cp -a $$component/build/* $@/; done

$(SUB_DIR):
	$(MAKE) -C $(SRC_DIR)/$@

zh_back:zh_pub zh_external

zh_front:zh_pub zh_external

clean:
	rm -rf $(DELIVER_PATH)
	for sub_component in $(SUB_DIR); do make clean -C $(SRC_DIR)/$$sub_component;done

.PHONY:all $(SUB_DIR) $(DELIVER_PATH) clean pack