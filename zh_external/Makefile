SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build
all:$(SRC_DIR)/*.c
	[ -d $(DELIVER_PATH) ] || mkdir $(DELIVER_PATH)
	gcc -o $(DELIVER_PATH)/hello_external $^

clean:
	rm -rf $(DELIVER_PATH)