<template>
<div class="system_management_show">
    <h2>当前版本: {{current_version}}</h2>

    <el-tabs v-model="activeName">
        <el-tab-pane label="设备配置" name="device_config">
            <div class="device_config_show" v-for="(single_gate, index) in device_config.gate" :key="'gate' + index">
                <el-descriptions :column="2" border :title="single_gate.name">
                    <el-descriptions-item label="入口编码">{{single_gate.entry}}</el-descriptions-item>
                    <el-descriptions-item label="出口编码">{{single_gate.exit}}</el-descriptions-item>
                    <el-descriptions-item label="入口身份识别IP">{{single_gate.entry_id_reader_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口身份识别IP">{{single_gate.exit_printer_ip}}</el-descriptions-item>
                    <template slot="extra">
                        <el-button type="success" size="small" @click="open_gate_operate(single_gate)">操作</el-button>
                        <el-button type="primary" size="small" @click="open_gate_edit(single_gate)">编辑</el-button>
                    </template>
                </el-descriptions>
            </div>
            <div class="device_config_show" v-for="(single_scale, index) in device_config.scale" :key="'scale' + index">
                <el-descriptions border :title="single_scale.name">
                    <el-descriptions-item label="入口编码">{{single_scale.entry}}</el-descriptions-item>
                    <el-descriptions-item label="出口编码">{{single_scale.exit}}</el-descriptions-item>
                    <el-descriptions-item label="衡器IP">{{single_scale.scale_ip}}</el-descriptions-item>
                    <el-descriptions-item label="光栅IP-1">{{single_scale.raster_ip[0]}}</el-descriptions-item>
                    <el-descriptions-item label="光栅IP-2">{{single_scale.raster_ip[1]}}</el-descriptions-item>
                    <el-descriptions-item label="入口打印机IP">{{single_scale.entry_printer_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口打印机IP">{{single_scale.exit_printer_ip}}</el-descriptions-item>
                    <el-descriptions-item label="入口身份识别IP">{{single_scale.entry_id_reader_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口身份识别IP">{{single_scale.exit_id_reader_ip}}</el-descriptions-item>
                    <template slot="extra">
                        <el-button type="success" size="small" @click="open_scale_operate(single_scale)">操作</el-button>
                        <el-button type="primary" size="small" @click="open_scale_edit(single_scale)">编辑</el-button>
                    </template>
                </el-descriptions>
            </div>
            <el-dialog @close="init_device_info" title="修改配置" :visible.sync="show_edit_gate_config" width="60%" @keyup.enter.native="edit_gate">
                <el-form :model="gate_for_edit" ref="edit_gate_form" :rules="rules" label-width="120px">
                    <el-form-item label="名称" prop="name">
                        <el-input v-model="gate_for_edit.name" placeholder="请输入名称"></el-input>
                    </el-form-item>
                    <el-form-item label="入口编码" prop="entry">
                        <el-input v-model="gate_for_edit.entry" placeholder="请输入入口编码"></el-input>
                    </el-form-item>
                    <el-form-item label="出口编码" prop="exit">
                        <el-input v-model="gate_for_edit.exit" placeholder="请输入出口编码"></el-input>
                    </el-form-item>
                    <el-form-item label="入口身份识别IP" prop="entry_id_reader_ip">
                        <el-input v-model="gate_for_edit.entry_id_reader_ip" placeholder="请输入入口身份识别IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口身份识别IP" prop="exit_id_reader_ip">
                        <el-input v-model="gate_for_edit.exit_id_reader_ip" placeholder="请输入出口身份识别IP"></el-input>
                    </el-form-item>
                    <el-form-item>
                        <el-button type="primary" @click="edit_gate">确认</el-button>
                    </el-form-item>
                </el-form>
            </el-dialog>
            <el-dialog @close="init_device_info" title="修改配置" :visible.sync="show_edit_scale_config" width="60%" @keyup.enter.native="edit_scale">
                <el-form :model="scale_for_edit" ref="edit_scale_form" :rules="rules" label-width="120px">
                    <el-form-item label="名称" prop="name">
                        <el-input v-model="scale_for_edit.name" placeholder="请输入名称"></el-input>
                    </el-form-item>
                    <el-form-item label="入口编码" prop="entry">
                        <el-input v-model="scale_for_edit.entry" placeholder="请输入入口编码"></el-input>
                    </el-form-item>
                    <el-form-item label="出口编码" prop="exit">
                        <el-input v-model="scale_for_edit.exit" placeholder="请输入出口编码"></el-input>
                    </el-form-item>
                    <el-form-item label="衡器IP" prop="scale_ip">
                        <el-input v-model="scale_for_edit.scale_ip" placeholder="请输入衡器IP"></el-input>
                    </el-form-item>
                    <el-form-item label="光栅IP-1" prop="raster_ip">
                        <el-input v-model="scale_for_edit.raster_ip[0]" placeholder="请输入光栅IP-1"></el-input>
                    </el-form-item>
                    <el-form-item label="光栅IP-2" prop="raster_ip">
                        <el-input v-model="scale_for_edit.raster_ip[1]" placeholder="请输入光栅IP-2"></el-input>
                    </el-form-item>
                    <el-form-item label="入口打印机IP" prop="entry_printer_ip">
                        <el-input v-model="scale_for_edit.entry_printer_ip" placeholder="请输入入口打印机IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口打印机IP" prop="exit_printer_ip">
                        <el-input v-model="scale_for_edit.exit_printer_ip" placeholder="请输入出口打印机IP"></el-input>
                    </el-form-item>
                    <el-form-item label="入口身份识别IP" prop="entry_id_reader_ip">
                        <el-input v-model="scale_for_edit.entry_id_reader_ip" placeholder="请输入入口身份识别IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口身份识别IP" prop="exit_id_reader_ip">
                        <el-input v-model="scale_for_edit.exit_id_reader_ip" placeholder="请输入出口身份识别IP"></el-input>
                    </el-form-item>
                    <el-form-item>
                        <el-button type="primary" @click="edit_scale">确认</el-button>
                    </el-form-item>
                </el-form>
            </el-dialog>
            <el-dialog :title="cur_opt_gate.name + '设备操作'" :visible.sync="show_gate_operate" width="60%">
                <el-row type="flex" :gutter="10">
                    <el-col :span="12">
                        <el-card>
                            <el-row type="flex" :gutter="10" align="center">
                                <el-col :span="10">入口身份证号</el-col>
                                <el-col :span="14">{{cur_opt_gate.entry_id_no}}</el-col>
                            </el-row>
                            <el-row type="flex" :gutter="10" align="center">
                                <el-col :span="10">出口身份证号</el-col>
                                <el-col :span="14">{{cur_opt_gate.exit_id_no}}</el-col>
                            </el-row>
                            <div slot="header">
                                <span>身份证阅读</span>
                                <el-button style="float: right; padding: 3px 3px" type="warning" @click="get_id_no">获取</el-button>
                            </div>
                        </el-card>
                    </el-col>
                    <el-col :span="12">
                        <gate-device-ctrl :entry="cur_opt_gate.entry" :exit="cur_opt_gate.exit"></gate-device-ctrl>
                    </el-col>
                </el-row>
                <el-row type="flex" :gutter="10">
                    <el-col :span="12">
                        <led-voice-ctrl device_name="入口播报显示设备" :gate_code="cur_opt_gate.entry"></led-voice-ctrl>
                    </el-col>
                    <el-col :span="12">
                        <led-voice-ctrl device_name="出口播报显示设备" :gate_code="cur_opt_gate.exit"></led-voice-ctrl>
                    </el-col>
                </el-row>
            </el-dialog>
            <el-dialog :title="cur_opt_scale.name + '设备操作'" :visible.sync="show_scale_operate" width="60%">
                <el-row type="flex" :gutter="10">
                    <el-col :span="8">
                        <el-card>
                            <el-row type="flex" :gutter="10" align="center">
                                <el-col :span="10">光栅1</el-col>
                                <el-col :span="14">{{cur_opt_scale.raster1_block?'阻挡':'未阻挡'}}</el-col>
                            </el-row>
                            <el-row type="flex" :gutter="10" align="center">
                                <el-col :span="10">光栅2</el-col>
                                <el-col :span="14">{{cur_opt_scale.raster2_block?'阻挡':'未阻挡'}}</el-col>
                            </el-row>
                            <el-row type="flex" :gutter="10" align="center">
                                <el-col :span="10">当前称重</el-col>
                                <el-col :span="14">{{cur_opt_scale.current_weight}}</el-col>
                            </el-row>
                            <div slot="header">
                                <span>光栅和衡器</span>
                                <el-button style="float: right; padding: 3px 3px" type="warning" @click="get_raster_status">获取</el-button>
                            </div>
                        </el-card>
                    </el-col>
                    <el-col :span="8">
                        <el-card>
                            <el-input type="textarea" :rows="5" placeholder="待打印内容" v-model="content_need_print1">
                            </el-input>
                            <div slot="header">
                                <span>入口打印机</span>
                                <el-button style="float: right; padding: 3px 3px" type="warning" @click="print_content1">打印</el-button>
                            </div>
                        </el-card>
                    </el-col>
                    <el-col :span="8">
                        <el-card>
                            <el-input type="textarea" :rows="5" placeholder="待打印内容" v-model="content_need_print2">
                            </el-input>
                            <div slot="header">
                                <span>出口打印机</span>
                                <el-button style="float: right; padding: 3px 3px" type="warning" @click="print_content2">打印</el-button>
                            </div>
                        </el-card>
                    </el-col>
                </el-row>
                <el-row type="flex" :gutter="10">
                    <el-col :span="8">
                        <gate-device-ctrl :entry="cur_opt_scale.entry" :exit="cur_opt_scale.exit"></gate-device-ctrl>
                    </el-col>
                    <el-col :span="8">
                        <led-voice-ctrl device_name="入口播报显示设备" :gate_code="cur_opt_scale.entry"></led-voice-ctrl>
                    </el-col>
                    <el-col :span="8">
                        <led-voice-ctrl device_name="出口播报显示设备" :gate_code="cur_opt_scale.exit"></led-voice-ctrl>
                    </el-col>
                </el-row>
                <el-card>
                    <el-row type="flex" :gutter="10" align="center">
                        <el-col :span="10">入口身份证号</el-col>
                        <el-col :span="14">{{cur_opt_scale.entry_id_no}}</el-col>
                    </el-row>
                    <el-row type="flex" :gutter="10" align="center">
                        <el-col :span="10">出口身份证号</el-col>
                        <el-col :span="14">{{cur_opt_scale.exit_id_no}}</el-col>
                    </el-row>
                    <div slot="header">
                        <span>身份证阅读</span>
                        <el-button style="float: right; padding: 3px 3px" type="warning" @click="get_id_no">获取</el-button>
                    </div>
                </el-card>
            </el-dialog>
        </el-tab-pane>
    </el-tabs>
</div>
</template>

<script>
import GateDeviceCtrl from '../components/GateDeviceCtrl.vue'
import LedVoiceCtrl from '../components/LedVoiceCtrl.vue'
export default {
    name: 'SystemManagement',
    components: {
        "gate-device-ctrl": GateDeviceCtrl,
        "led-voice-ctrl": LedVoiceCtrl,
    },
    data: function () {
        return {
            content_need_print1: '',
            content_need_print2: '',
            cur_opt_scale: {},
            cur_opt_gate: {},
            show_scale_operate: false,
            show_gate_operate: false,
            activeName: 'device_config',
            current_version: '',
            device_config: {},
            gate_for_edit: {},
            scale_for_edit: {
                raster_ip: ['', '']
            },
            rules: {
                name: [{
                    required: true,
                    message: "请输入名称"
                }],
                entry: [{
                    required: true,
                    message: "请输入入口编码"
                }],
                exit: [{
                    required: true,
                    message: "请输入出口编码"
                }],
                entry_id_reader_ip: [{
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                exit_id_reader_ip: [{
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                scale_ip: [{
                    required: true,
                    message: "请输入衡器IP"
                }, {
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                raster_ip: [{
                    required: true,
                    message: "请输入光栅IP"
                }, {
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                entry_printer_ip: [{
                    required: true,
                    message: "请输入打印机IP"
                }, {
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                exit_printer_ip: [{
                    required: true,
                    message: "请输入打印机IP"
                }, {
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
            },
            show_edit_gate_config: false,
            show_edit_scale_config: false,
        };
    },
    methods: {
        open_gate_operate: function (_gate) {
            this.cur_opt_gate = {
                ..._gate
            };
            this.show_gate_operate = true;
        },
        get_id_no: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "read_id_no", [vue_this.cur_opt_gate.entry_id_reader_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_gate, "entry_id_no", resp);
            });
            vue_this.$call_remote_process("system_management", "read_id_no", [vue_this.cur_opt_gate.exit_id_reader_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_gate, "entry_id_no", resp);
            });
            vue_this.$call_remote_process("system_management", "read_id_no", [vue_this.cur_opt_scale.entry_id_reader_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, "entry_id_no", resp);
            });
            vue_this.$call_remote_process("system_management", "read_id_no", [vue_this.cur_opt_scale.exit_id_reader_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, "exit_id_no", resp);
            });
        },
        print_content1: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "print_content", [vue_this.cur_opt_scale.entry_printer_ip, vue_this.content_need_print1]).then(function (resp) {
                if (resp) {
                    vue_this.$message({
                        message: "打印成功",
                        type: 'success',
                    });
                } else {
                    vue_this.$message({
                        message: "打印失败",
                        type: 'error',
                    });
                }
            });
        },
        print_content2: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "print_content", [vue_this.cur_opt_scale.exit_printer_ip, vue_this.content_need_print2]).then(function (resp) {
                if (resp) {
                    vue_this.$message({
                        message: "打印成功",
                        type: 'success',
                    });
                } else {
                    vue_this.$message({
                        message: "打印失败",
                        type: 'error',
                    });
                }
            });
        },
        get_raster_status: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "raster_is_block", [vue_this.cur_opt_scale.raster_ip[0]]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, 'raster1_block', resp);
            });
            vue_this.$call_remote_process("system_management", "raster_is_block", [vue_this.cur_opt_scale.raster_ip[1]]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, 'raster2_block', resp);
            });
            vue_this.$call_remote_process("system_management", "read_scale", [vue_this.cur_opt_scale.scale_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, 'current_weight', resp);
            });
        },
        open_scale_operate: function (_scale) {
            this.show_scale_operate = true;
            this.cur_opt_scale = {
                ..._scale
            };
        },
        edit_gate: function () {
            var vue_this = this;
            this.$refs.edit_gate_form.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("system_management", "edit_device_config", [vue_this.$cookies.get("zh_ssid"), vue_this.device_config]).then(function (resp) {
                        if (resp) {
                            vue_this.show_edit_gate_config = false;
                        }
                    });
                }
            });
        },
        edit_scale: function () {
            var vue_this = this;
            vue_this.$refs.edit_scale_form.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("system_management", "edit_device_config", [vue_this.$cookies.get("zh_ssid"), vue_this.device_config]).then(function (resp) {
                        if (resp) {
                            vue_this.show_edit_scale_config = false;
                        }
                    });
                }
            });
        },
        open_gate_edit: function (_gate) {
            this.gate_for_edit = _gate;
            this.show_edit_gate_config = true;
        },
        open_scale_edit: function (_scale) {
            this.scale_for_edit = _scale;
            this.show_edit_scale_config = true;
        },
        init_device_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", 'get_device_config', [vue_this.$cookies.get('zh_ssid')]).then(function (resp) {
                vue_this.device_config = resp;
            });
        },
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("system_management", 'current_version', []).then(function (resp) {
            vue_this.current_version = resp;
        });
        this.init_device_info();
    },
}
</script>

<style scoped>
.device_config_show {
    border: 1px solid black;
    margin-top: 10px;
    padding: 5px;
}
</style>
