SHELL=/bin/bash
SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
DELIVER_PATH=$(SRC_DIR)/build

all:$(DELIVER_PATH)/bin/zh_sprt_printer

$(DELIVER_PATH):
	[ -d $(DELIVER_PATH) ] || mkdir $(DELIVER_PATH)
	[ -d $(DELIVER_PATH)/bin ] || mkdir $(DELIVER_PATH)/bin
	[ -d $(DELIVER_PATH)/lib ] || mkdir $(DELIVER_PATH)/lib

$(DELIVER_PATH)/bin/zh_sprt_printer:*.c $(DELIVER_PATH)
	gcc -m32 -g -o $@ *.c -L $(DELIVER_PATH)/lib -l HWISPRTPrinter -l usb-1.0 -lrt -pthread
clean:
	rm -rf $(DELIVER_PATH)

.PHONY:all clean