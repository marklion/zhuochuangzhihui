SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build
all:$(DELIVER_PATH)/zh_rest_node

$(DELIVER_PATH):
	[ -d $(DELIVER_PATH) ] || mkdir $(DELIVER_PATH)

$(DELIVER_PATH)/zh_rest_node:$(DELIVER_PATH) $(SRC_DIR)/index.js $(SRC_DIR)/package.json
	[ -d $(DELIVER_PATH)/zh_rest_node ] || mkdir $(DELIVER_PATH)/zh_rest_node
	cp -a $(SRC_DIR)/node_modules $(DELIVER_PATH)/zh_rest_node/
	cp -a $(SRC_DIR)/gen_code $(DELIVER_PATH)/zh_rest_node/
	cp $(SRC_DIR)/index.js $(DELIVER_PATH)/zh_rest_node/
	cp $(SRC_DIR)/package-lock.json $(DELIVER_PATH)/zh_rest_node/
	cp $(SRC_DIR)/package.json $(DELIVER_PATH)/zh_rest_node/
	pushd $(DELIVER_PATH)/zh_rest_node; npm install
clean:
	rm -rf $(DELIVER_PATH)

.PHONY:all $(DELIVER_PATH) clean