SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
OUT_CODE_PATH=$(SRC_DIR)/gen_code

all:$(OUT_CODE_PATH)
	mkdir -p $(SRC_DIR)/../../zh_front/zh_manage/public/gen_code
	mkdir -p $(SRC_DIR)/../../zh_back/zh_rest_server/gen_code
	cp $(OUT_CODE_PATH)/js/* $(SRC_DIR)/../../zh_front/zh_manage/public/gen_code/
	cp $(OUT_CODE_PATH)/node/* $(SRC_DIR)/../../zh_back/zh_rest_server/gen_code/

$(OUT_CODE_PATH):$(OUT_CODE_PATH)/cpp $(OUT_CODE_PATH)/js $(OUT_CODE_PATH)/node

$(OUT_CODE_PATH)/cpp:$(SRC_DIR)/idl.thrift
	[ -d $@ ] || mkdir -p $@
	thrift -out $@ --gen cpp:no_skeleton $^

$(OUT_CODE_PATH)/js:$(SRC_DIR)/idl.thrift
	[ -d $@ ] || mkdir -p $@
	thrift -out $@ --gen js:es6 $^

$(OUT_CODE_PATH)/node:$(SRC_DIR)/idl.thrift
	[ -d $@ ] || mkdir -p $@
	thrift -out $@ --gen js:node $^

clean:
	rm -rf $(OUT_CODE_PATH)/*

.PHONY: all clean