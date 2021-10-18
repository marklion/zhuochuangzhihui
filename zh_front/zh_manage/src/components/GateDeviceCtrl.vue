<template>
<div class="gate_device_ctrl_show">
    <el-card>
        <div class="road_block_show">
            <el-row type="flex" :gutter="10" align="center">
                <el-col :span="6">入口</el-col>
                <el-col :span="6">
                    <el-button size="small" type="primary" @click="ctrl_gate(1, entry)">开闸</el-button>
                </el-col>
                <el-col :span="6">
                    <el-button size="small" type="danger" @click="ctrl_gate(0, entry)">关闸</el-button>
                </el-col>
                <el-col :span="6">
                    <el-button size="small" type="success" @click="ctrl_gate(2, entry)">常开</el-button>
                </el-col>
            </el-row>
            <div>{{entry_status.coming_vehicle?'抓拍车辆:' + entry_status.coming_vehicle:'未抓拍到车辆'}}</div>
        </div>
        <div class="road_block_show">
            <el-row type="flex" :gutter="10" align="center">
                <el-col :span="6">出口</el-col>
                <el-col :span="6">
                    <el-button size="small" type="primary" @click="ctrl_gate(1, exit)">开闸</el-button>
                </el-col>
                <el-col :span="6">
                    <el-button size="small" type="danger" @click="ctrl_gate(0, exit)">关闸</el-button>
                </el-col>
                <el-col :span="6">
                    <el-button size="small" type="success" @click="ctrl_gate(2, exit)">常开</el-button>
                </el-col>
            </el-row>
            <div>{{exit_status.coming_vehicle?'抓拍车辆:' + exit_status.coming_vehicle:'未抓拍到车辆'}}</div>
        </div>

        <div slot="header">
            <span>道闸控制</span>
            <el-button style="float: right; padding: 3px 3px" type="warning" @click="get_road_status">手动检测</el-button>
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
    },
    data: function () {
        return {
            entry_status: {
                coming_vehicle: '',
            },
            exit_status: {
                coming_vehicle: '',
            },
        };
    },
    methods: {
        get_road_status: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_road_status", [vue_this.entry]).then(function (resp) {
                vue_this.entry_status = resp;
            });
            vue_this.$call_remote_process("system_management", "get_road_status", [vue_this.exit]).then(function (resp) {
                vue_this.exit_status = resp;
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
