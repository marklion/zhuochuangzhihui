<template>
<div class="zh_led_plan_call">
    <span>屏幕IP</span>
    <el-input v-model="led_ip"></el-input>
    <span>计划容量</span>
    <el-input v-model="plan_count"></el-input>
    <span>叫号容量</span>
    <el-input v-model="call_count"></el-input>
    <el-button type="primary" @click="save_param" size="small">保存</el-button>
</div>
</template>

<script>
export default {
    name: 'zh_led_plan_call',
    data: function () {
        return {
            led_ip: '',
            plan_count: '',
            call_count: '',
        };
    },
    methods: {
        init_data: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_led_plan_call', 'get -k led_ip']).then(function (resp) {
                if (resp) {
                    vue_this.led_ip = resp;
                }
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_led_plan_call', 'get -k plan_count']).then(function (resp) {
                if (resp) {
                    vue_this.plan_count = resp;
                }
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_led_plan_call', 'get -k call_count']).then(function (resp) {
                if (resp) {
                    vue_this.call_count = resp;
                }
            });

        },
        save_param: async function () {
            var vue_this = this;
            try {
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_led_plan_call', 'set -k led_ip ' + vue_this.led_ip]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_led_plan_call', 'set -k plan_count ' + vue_this.plan_count]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_led_plan_call', 'set -k call_count ' + vue_this.call_count]);
                vue_this.init_data();
            } catch (error) {
                console.log(error);
            }
        },
    },
    beforeMount: function () {
        this.init_data();
    }
}
</script>

<style>

</style>
