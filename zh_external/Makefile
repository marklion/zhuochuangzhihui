SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build
SUB_DIR=printer

all:$(DELIVER_PATH)

$(DELIVER_PATH):$(SUB_DIR)
	[ -d $@ ] || mkdir $@
	for component in $^;do [ -d $$component/build ] && cp -a $$component/build/* $@/; done

$(SUB_DIR):
	$(MAKE) -C $(SRC_DIR)/$@
clean:
	rm -rf $(DELIVER_PATH)
	for sub_component in $(SUB_DIR); do make clean -C $(SRC_DIR)/$$sub_component;done

.PHONY:all $(SUB_DIR) $(DELIVER_PATH) clean