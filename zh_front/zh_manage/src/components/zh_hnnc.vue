<template>
<div class="zh_hnnc_show">
    <span>公司简称：</span>
    <el-input v-model="corp_name"></el-input>
    <span>公司编码：{{corp_code}}</span>
    <el-button type="primary" @click="save_corp_name" size="small">保存</el-button>
    <el-button v-if="corp_code" type="success" @click="sync_stuff_info" size="small">同步物料信息</el-button>
    <el-button v-if="corp_code" type="success" @click="sync_customer_info" size="small">同步销售信息</el-button>
    <el-button v-if="corp_code" type="success" @click="sync_supplier_info" size="small">同步采购信息</el-button>
</div>
</template>

<script>
export default {
    name: "zh_hnnc",
    data: function () {
        return {
            corp_name: '',
            corp_code: '',
        };
    },
    methods: {
        init_corp_data: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "get -k corp_name"]).then(function (resp) {
                if (resp) {
                    vue_this.corp_name = resp;
                    vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "fetchCorp"]).then(function (resp) {
                        var all_corp = JSON.parse(resp);
                        all_corp.forEach(element => {
                            if (element.unitshortname == vue_this.corp_name) {
                                vue_this.corp_code = element.corpcode;
                                vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "set -k corpcode " + vue_this.corp_code]);
                            }
                        });
                    });
                }
            });
        },
        save_corp_name: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "set -k corp_name " + vue_this.corp_name]).then(function () {
                vue_this.init_corp_data();
            })
        },
        sync_supplier_info: async function () {
            var vue_this = this;
            try {
                var contract_info = await vue_this.$call_remote_process("contract_management", "get_all_contract", [vue_this.$cookies.get("zh_ssid")]);
                vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "fetchSupplier"]).then(function (resp) {
                    var customer_info = JSON.parse(resp);
                    customer_info.forEach(element => {
                        vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "fetchSupplyOrder " + element.custcode]).then(function (all_order_resp) {
                            var got_fs = [];
                            JSON.parse(all_order_resp).forEach(order_resp => {
                                got_fs.push(order_resp.invname);
                            });
                            var found_contract = contract_info.find(single_contract => {
                                return (single_contract.name == element.custname) && !single_contract.is_sale;
                            });
                            if (!found_contract) {
                                var new_contract = {
                                    name: element.custname,
                                    is_sale: false,
                                    code: element.custcode,
                                    follow_stuffs: got_fs,
                                };
                                vue_this.$call_remote_process("contract_management", "add_contract", [vue_this.$cookies.get("zh_ssid"), new_contract]);
                            } else {
                                found_contract.follow_stuffs = got_fs;
                                found_contract.code = element.custcode;
                                vue_this.$call_remote_process("contract_management", "update_contract", [vue_this.$cookies.get("zh_ssid"), found_contract]);
                            }
                        });
                    });
                });
            } catch (error) {
                console.log(error);
            }
        },
        sync_customer_info: async function () {
            var vue_this = this;
            try {
                var contract_info = await vue_this.$call_remote_process("contract_management", "get_all_contract", [vue_this.$cookies.get("zh_ssid")]);
                vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "fetchCust"]).then(function (resp) {
                    var customer_info = JSON.parse(resp);
                    vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "set -k customers '" + resp + "' -o"]);
                    customer_info.forEach(element => {
                        vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "fetchOrder " + element.custcode]).then(function (all_order_resp) {
                            var got_fs = [];
                            JSON.parse(all_order_resp).forEach(order_resp => {
                                got_fs.push(order_resp.invname);
                            });
                            var found_contract = contract_info.find(single_contract => {
                                return (single_contract.name == element.custname) && single_contract.is_sale;
                            });
                            if (!found_contract) {
                                var new_contract = {
                                    name: element.custname,
                                    is_sale: true,
                                    code: element.custcode,
                                    follow_stuffs: got_fs,
                                };
                                vue_this.$call_remote_process("contract_management", "add_contract", [vue_this.$cookies.get("zh_ssid"), new_contract]);
                            } else {
                                found_contract.follow_stuffs = got_fs;
                                found_contract.code = element.custcode;
                                vue_this.$call_remote_process("contract_management", "update_contract", [vue_this.$cookies.get("zh_ssid"), found_contract]);
                            }
                        });
                    });
                });
            } catch (error) {
                console.log(error);
            }
        },
        sync_stuff_info: async function () {
            var vue_this = this;
            try {
                var stuff_info = await vue_this.$call_remote_process("stuff_management", "get_all_stuff", [vue_this.$cookies.get("zh_ssid")]);
                var set_stuff_func = function (all_order_resp) {
                    JSON.parse(all_order_resp).forEach(order_resp => {
                        var found_stuff = stuff_info.find(single_stuff => {
                            return single_stuff.name == order_resp.invname;
                        });
                        if (!found_stuff) {
                            var new_stuff = {
                                name: order_resp.invname,
                                unit: '吨',
                                code: order_resp.invcode,
                            };
                            vue_this.$call_remote_process("stuff_management", "add_stuff", [vue_this.$cookies.get("zh_ssid"), new_stuff]);
                        } else {
                            found_stuff.code = order_resp.invcode;
                            vue_this.$call_remote_process("stuff_management", "update_stuff", [vue_this.$cookies.get("zh_ssid"), found_stuff]);
                        }
                    });
                };
                vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "fetchCust"]).then(function (resp) {
                    var customer_info = JSON.parse(resp);
                    vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "set -k customers '" + resp + "' -o"]);
                    customer_info.forEach(element => {
                        vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "fetchOrder " + element.custcode]).then(function (all_order_resp) {
                            set_stuff_func(all_order_resp);
                        });
                    });
                });
                vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "fetchSupplier"]).then(function (resp) {
                    var supplier_info = JSON.parse(resp);
                    console.log(supplier_info);
                    supplier_info.forEach(element => {
                        vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_hnnc", "fetchSupplyOrder " + element.custcode]).then(function (all_order_resp) {
                            set_stuff_func(all_order_resp);
                        });
                    });
                });
            } catch (error) {
                console.log(error);
            }

        },
    },
    beforeMount: function () {
        this.init_corp_data();
    }
}
</script>

<style>

</style>
