SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
SUB_DIR=zh_rpc_base

all:$(SUB_DIR)

$(SUB_DIR):
	make -C $(SRC_DIR)/$@

clean:
	for sub_component in $(SUB_DIR); do make clean -C $(SRC_DIR)/$$sub_component;done

.PHONY:all clean $(SUB_DIR)