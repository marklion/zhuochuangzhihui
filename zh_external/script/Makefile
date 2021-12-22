SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build

all:$(DELIVER_PATH)

$(DELIVER_PATH):
	[ -d $(DELIVER_PATH) ] || mkdir $(DELIVER_PATH)
	[ -d $(DELIVER_PATH)/script ] || mkdir $(DELIVER_PATH)/script
	cp -a $(SRC_DIR)/src/* $(DELIVER_PATH)/script/
	chmod +x $(DELIVER_PATH)/script/*

clean:
	rm -rf $(DELIVER_PATH)

.PHONY:all clean $(DELIVER_PATH)