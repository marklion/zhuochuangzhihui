SRC_DIR:=$(shell dirname $(realpath $(lastword $(MAKEFILE_LIST))))
CONF_DELIVER_PATH=$(SRC_DIR)/build/conf
SUB_DIR=zh_rpc_base

all:$(CONF_DELIVER_PATH) $(SUB_DIR)
	cp $(SRC_DIR)/zh_conf/nginx.conf $(CONF_DELIVER_PATH)
	cp $(SRC_DIR)/zh_conf/frpc.ini $(CONF_DELIVER_PATH)
	cp $(SRC_DIR)/zh_conf/change_base_url.sh $(CONF_DELIVER_PATH)

$(SUB_DIR):
	$(MAKE) -C $(SRC_DIR)/$@

$(CONF_DELIVER_PATH):
	[ -d $(CONF_DELIVER_PATH) ] || mkdir -p $(CONF_DELIVER_PATH)

clean:
	for sub_component in $(SUB_DIR); do make clean -C $(SRC_DIR)/$$sub_component;done

.PHONY:all clean $(SUB_DIR)