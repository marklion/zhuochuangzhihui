SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build
all:$(DELIVER_PATH)/manage_dist

$(DELIVER_PATH):
	[ -d $(DELIVER_PATH) ] || mkdir $(DELIVER_PATH)

$(DELIVER_PATH)/manage_dist:$(DELIVER_PATH)
	rm -rf $@
	pushd $(SRC_DIR)/zh_manage;[ -d node_modules ] || npm install; npm run build; cp ./dist -a $@; popd

clean:
	rm -rf $(DELIVER_PATH)

.PHONY:all $(DELIVER_PATH)/manage_dist