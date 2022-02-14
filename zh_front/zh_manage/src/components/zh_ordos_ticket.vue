<template>
<div class="plugin_ordos_ticket_show">
    <el-form ref="login_form" :model="login_form" v-if="!is_login" label-width="80px">
        <el-form-item label="用户名">
            <el-input v-model="login_form.username"></el-input>
        </el-form-item>
        <el-form-item label="密码">
            <el-input v-model="login_form.password"></el-input>
        </el-form-item>
        <el-form-item label="验证码">
            <span>
                <img :src="'data:image/png;base64,' + verify_code" />
            </span>
            <el-button type="text" @click="get_verify_code">换一个</el-button>
            <el-input v-model="login_form.verify_code"></el-input>
        </el-form-item>
        <el-form-item>
            <el-button type="primary" @click="login">登录</el-button>
        </el-form-item>
    </el-form>
    <div v-else>
        <el-form :model="runtime_config" label-width="80px">
            <el-form-item label="打印机地址URI">
                <el-input v-model="runtime_config.device_uri"></el-input>
                <el-button size="small" type="primary" @click="save_printer">保存</el-button>
                <el-button size="small" type="success" @click="print_ticket_diag = true">手动打票</el-button>
            </el-form-item>
            <el-form-item label="GPS数据">
                <el-input v-model="runtime_config.gps"></el-input>
                <el-button size="small" type="primary" @click="save_gps">保存</el-button>
            </el-form-item>
            <el-form-item label="自动开票">
                <el-switch v-model="runtime_config.auto_print" @change="change_auto_switch"></el-switch>
            </el-form-item>
            <el-form-item label="当前票本" v-if="runtime_config.auto_print">
                <el-input v-model="cur_tid" disabled></el-input>
                <el-popover placement="bottom" title="可用票本" width="800" trigger="click" v-model="choose_ticket_show">
                    <el-button slot="reference" type="text">选择</el-button>
                    <el-table :data="tids">
                        <el-table-column prop="WarehouseID" label="票本ID" min-width="180">
                        </el-table-column>
                        <el-table-column prop="StartNumber" label="起始编号" min-width="180">
                        </el-table-column>
                        <el-table-column prop="EndNumber" label="结束编号" min-width="180">
                        </el-table-column>
                        <el-table-column label="选择">
                            <template slot-scope="scope">
                                <el-button @click="choose(scope.row)" type="text" size="small">选择</el-button>
                            </template>
                        </el-table-column>
                    </el-table>
                </el-popover>
            </el-form-item>
        </el-form>
    </div>
    <el-dialog @close="clean_print_req" title="手动打票" :visible.sync="print_ticket_diag" width="60%" @keyup.enter.native="print_ticket">
        <el-form :model="print_req" ref="print_req_form" :rules="print_req_rules" label-width="150px">
            <el-form-item v-for="(single_req_item, index) in print_req_array()" :key="index" :label="single_req_item.doc" :prop="single_req_item.name">
                <el-input v-model="print_req[single_req_item.name]" :placeholder="'请输入'+single_req_item.doc"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="print_ticket">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</div>
</template>

<script>
export default {
    name: 'zh_ordos_ticket',
    data: function () {
        return {
            print_req_array: function () {
                var ret = [];
                Object.keys(this.print_req).forEach((element) => {
                    ret.push({
                        name: element,
                        value: this.print_req[element],
                        doc: this.print_arg_doc[element],
                    });
                });
                return ret;
            },
            print_arg_doc: {
                "VehicleNum": "车牌号",
                "EmptyWeight": "皮重",
                "GrossWeight": "毛重",
                "Axes": "车轴数",
                "CurbWeight": "净重",
                "CurbWeightDateTime": "称重时间",
                "ProductID": "产品编号",
                "SinglePrice": "单价",
                "CustomerID": "客户编号",
                "DivisionID": "收货地点编号",
                "DivisionName": "收货地点",
                "DivisionDetail": "收货详细地址",
                "WarehouseDetail": "票号",
                "ClearInventory": "保留",
                "GuarantHeat": "保留",
                "Category": "用途编号"
            },
            print_req: {
                "VehicleNum": "",
                "EmptyWeight": "",
                "GrossWeight": "",
                "Axes": "",
                "CurbWeight": "",
                "CurbWeightDateTime": "",
                "ProductID": "",
                "SinglePrice": "",
                "CustomerID": "",
                "DivisionID": "",
                "DivisionName": "",
                "DivisionDetail": "",
                "WarehouseDetail": "",
                "ClearInventory": "",
                "GuarantHeat": "",
                "Category": ""
            },
            print_ticket_diag: false,
            choose_ticket_show: false,
            runtime_config: {
                auto_print: false,
                device_uri: '',
                gps:'',
            },
            verify_code: "",
            is_login: false,
            tids: [],
            login_form: {
                username: '',
                password: '',
                verify_code: '',
            },
            cur_tid: "",
            print_req_rules: {
                WarehouseDetail: [{
                    required: true,
                    message: '请输入票号',
                    trigger: 'blur'
                }, ],
            },
        };
    },
    methods: {
        print_ticket: function () {
            var vue_this = this;
            vue_this.$refs.print_req_form.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "print '" + JSON.stringify(vue_this.print_req) + "'"]).then(function () {
                        vue_this.print_ticket_diag = false;
                    });
                }
            });
        },
        clean_print_req: function () {
            this.print_req = {
                "VehicleNum": "",
                "EmptyWeight": "",
                "GrossWeight": "",
                "Axes": "",
                "CurbWeight": "",
                "CurbWeightDateTime": "",
                "ProductID": "",
                "SinglePrice": "",
                "CustomerID": "",
                "DivisionID": "",
                "DivisionName": "",
                "DivisionDetail": "",
                "WarehouseDetail": "",
                "ClearInventory": "",
                "GuarantHeat": "",
                "Category": ""
            };
        },
        init_device: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "get -k device_uri"]).then(function (resp) {
                vue_this.runtime_config.device_uri = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "get -k gps"]).then(function (resp) {
                vue_this.runtime_config.gps = resp;
            });
        },
        save_printer: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "set -k device_uri '" + vue_this.runtime_config.device_uri + "'"]).then(function () {
                vue_this.$message("保存成功");
                vue_this.init_device();
            });
        },
        save_gps: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "set -k gps '" + vue_this.runtime_config.gps+ "'"]).then(function () {
                vue_this.$message("保存成功");
                vue_this.init_device();
            });
        },
        change_auto_switch: function (_value) {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "set -k auto_switch " + (_value ? 'true' : 'false')]).then(function () {
                vue_this.init_tids();
            });
        },
        choose: function (_value) {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "set -k TID " + _value.WarehouseID]).then(function () {
                vue_this.choose_ticket_show = false;
                vue_this.init_tids();
            });
        },
        init_tids: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "tids"]).then(function (resp) {
                var resp_json = JSON.parse(resp);
                vue_this.tids = [];
                resp_json.forEach((element, index) => {
                    vue_this.$set(vue_this.tids, index, element);
                    vue_this.is_login = true;
                });
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "get -k TID"]).then(function (resp) {
                vue_this.cur_tid = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "get -k auto_switch"]).then(function (resp) {
                vue_this.runtime_config.auto_print = resp == "true" ? true : false;
            });
        },
        login: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "login -u " + vue_this.login_form.username + " -p " + vue_this.login_form.password + " -v " + vue_this.login_form.verify_code]).then(function () {
                vue_this.init_tids();
            });
        },
        get_verify_code: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "login -g"]).then(function (resp) {
                vue_this.verify_code = resp;
            });
        },
    },
    beforeMount: function () {
        this.get_verify_code();
        this.init_tids();
        this.init_device();
    }
}
</script>

<style>

</style>
