<template>
<div class="gate_device_ctrl_show">
    <el-card>
        <div class="road_block_show">
            <div>入口{{entry_close?'已关闸':'未关闸'}}</div>
            <el-row type="flex" :gutter="10" align="center">
                <el-col :span="4">入口</el-col>
                <el-col :span="5">
                    <el-button size="small" type="primary" @click="ctrl_gate(1, entry)">开闸</el-button>
                </el-col>
                <el-col :span="5">
                    <el-button size="small" type="danger" @click="ctrl_gate(0, entry)">关闸</el-button>
                </el-col>
                <el-col :span="5">
                    <el-button size="small" type="success" @click="ctrl_gate(2, entry)">常开</el-button>
                </el-col>
                <el-col :span="5">
                    <el-button size="small" type="warning" @click="cast_welcome(entry_led)">播报测试</el-button>
                </el-col>
            </el-row>
        </div>
        <div class="road_block_show">
            <div>出口{{exit_close?'已关闸':'未关闸'}}</div>
            <el-row type="flex" :gutter="10" align="center">
                <el-col :span="4">出口</el-col>
                <el-col :span="5">
                    <el-button size="small" type="primary" @click="ctrl_gate(1, exit)">开闸</el-button>
                </el-col>
                <el-col :span="5">
                    <el-button size="small" type="danger" @click="ctrl_gate(0, exit)">关闸</el-button>
                </el-col>
                <el-col :span="5">
                    <el-button size="small" type="success" @click="ctrl_gate(2, exit)">常开</el-button>
                </el-col>
                <el-col :span="5">
                    <el-button size="small" type="warning" @click="cast_welcome(exit_led)">播报测试</el-button>
                </el-col>
            </el-row>
        </div>

        <div slot="header">
            <span>抓拍和闸控</span>
            <el-button style="float: right; padding: 3px 3px" type="warning" @click="get_road_status">检测</el-button>
        </div>
    </el-card>
</div>
</template>

<script>
export default {
    name: 'GateDeviceCtrl',
    props: {
        entry: String,
        exit: String,
        entry_led: String,
        exit_led: String,
    },
    data: function () {
        return {
            entry_close: false,
            exit_close: false,
        };
    },
    methods: {
        cast_welcome: function (_led_ip) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "led_cast_welcome", [_led_ip]).then(function (resp) {
                if (resp) {
                    vue_this.$message({
                        message: '操作成功',
                        type: 'success',
                    });
                } else {
                    vue_this.$message({
                        message: '操作失败',
                        type: 'error',
                    });
                }
            });
        },
        get_road_status: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "read_cam_io", [vue_this.entry]).then(function (resp) {
                vue_this.entry_close = resp;
            });
            vue_this.$call_remote_process("system_management", "read_cam_io", [vue_this.exit]).then(function (resp) {
                vue_this.exit_close = resp;
            });
        },
        ctrl_gate: function (_cmd, gate_code) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", 'ctrl_gate', [gate_code, _cmd]).then(function (resp) {
                if (resp) {
                    vue_this.$message({
                        message: '操作成功',
                        type: 'success',
                    });
                } else {
                    vue_this.$message({
                        message: '操作失败',
                        type: 'error',
                    });
                }
            });
        }
    },
}
</script>

<style scoped>
.road_block_show {
    border: 1px solid;
    margin-top: 10px;
    padding: 10px;
}
</style>
