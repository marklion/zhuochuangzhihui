SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/plugin_build
SUB_DIR=zh_ordos_ticket zh_hnnc zh_meiyitong zh_led_plan_call zh_test_plugin zh_zyzl zh_zyhl zh_ods_nyj

all:$(DELIVER_PATH)
	for single_plugin in $(SUB_DIR); do tar zcf $(DELIVER_PATH)/$$single_plugin.tar.gz -C $(DELIVER_PATH) $$single_plugin; done

$(DELIVER_PATH):$(SUB_DIR)
	[ -d $@ ] || mkdir $@
	for component in $^;do [ -d $$component/build ] && [ -d $@/$$component ] || mkdir $@/$$component && cp -a $$component/build/* $@/$$component; done

$(SUB_DIR):
	$(MAKE) -C $(SRC_DIR)/$@
clean:
	rm -rf $(DELIVER_PATH)
	for sub_component in $(SUB_DIR); do make clean -C $(SRC_DIR)/$$sub_component;done

.PHONY:all $(SUB_DIR) $(DELIVER_PATH) clean