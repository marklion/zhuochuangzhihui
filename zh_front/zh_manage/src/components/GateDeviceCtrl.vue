<template>
<div class="gate_device_ctrl_show">
    <el-card>
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

        <div slot="header">
            <span>道闸控制</span>
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

        };
    },
    methods: {
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

<style>

</style>
