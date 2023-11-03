SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build
all:$(DELIVER_PATH)

$(DELIVER_PATH):
	pushd $(SRC_DIR); npm run build;

clean:
	rm -rf $(DELIVER_PATH)

.PHONY:all $(DELIVER_PATH) clean