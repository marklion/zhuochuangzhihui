<template>
<div class="led_voice_ctrl_show">
    <el-card>
        <el-input type="textarea" :rows="5" placeholder="待显示或播报内容" v-model="led_voice_content">
        </el-input>
        <div slot="header">
            <span>{{device_name}}</span>
            <el-button style="float: right; padding: 3px 3px" type="primary" @click="ctrl_led">显示</el-button>
            <el-button style="float: right; padding: 3px 3px" type="success" @click="ctrl_voice">播报</el-button>
        </div>
    </el-card>
</div>
</template>

<script>
export default {
    name: 'LedVoiceCtrl',
    props: {
        gate_code: String,
        device_name:String,
    },
    data: function () {
        return {
            led_voice_content: '',
        };
    },
    methods: {
        ctrl_led: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", 'ctrl_led', [vue_this.gate_code, vue_this.led_voice_content]).then(function (resp) {
                if (resp) {
                    vue_this.$message({
                        message: '显示成功',
                        type: 'success',
                    });
                } else {
                    vue_this.$message({
                        message: '显示失败',
                        type: 'error',
                    });
                }
            });
        },
        ctrl_voice: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", 'ctrl_voice', [vue_this.gate_code, vue_this.led_voice_content]).then(function (resp) {
                if (resp) {
                    vue_this.$message({
                        message: '播报成功',
                        type: 'success',
                    });
                } else {
                    vue_this.$message({
                        message: '播报失败',
                        type: 'error',
                    });
                }
            });
        },

    },
    beforeMount: function () {
        this.led_voice_content = '';
    },
}
</script>

<style>

</style>
