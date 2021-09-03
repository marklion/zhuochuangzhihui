SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build
all:$(DELIVER_PATH)/manage_dist

$(DELIVER_PATH):
	[ -d $(DELIVER_PATH) ] || mkdir $(DELIVER_PATH)

$(DELIVER_PATH)/manage_dist:$(DELIVER_PATH)
	if [ "$(ONLY_BACK_END)" != "y" ]; then rm -rf $@; pushd $(SRC_DIR)/zh_manage;[ -d node_modules ] || npm install; npm run $(BUILD_MODE); cp ./dist -a $@; popd; fi

clean:
	rm -rf $(DELIVER_PATH)

.PHONY:all $(DELIVER_PATH)/manage_dist