<template>
<div class="zh_zyhl_show">
    <span>远端地址</span>
    <el-input v-model="zy_url"></el-input>
    <span>appid</span>
    <el-input v-model="appid"></el-input>
    <span>secret</span>
    <el-input v-model="secret"></el-input>
    <el-button type="primary" @click="save_param" size="small">保存</el-button>
</div>
</template>

<script>
export default {
    name: "zh_zyhl",
    data: function () {
        return {
            zy_url: '',
            appid: '',
            secret: '',
        };
    },
    methods: {
        init_param: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'get -k zyhl_url']).then(function (resp) {
                vue_this.zy_url = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'get -k appid']).then(function (resp) {
                vue_this.appid = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'get -k secret']).then(function (resp) {
                vue_this.secret = resp;
            });
        },
        save_param: async function () {
            var vue_this = this;
            try {
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'set -k zyhl_url ' + vue_this.zy_url]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'set -k appid ' + vue_this.appid]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'set -k secret ' + vue_this.secret]);
            } catch (error) {
                console.log(error);
            }
        },
    },
    beforeMount: function () {
        this.init_param();
    }
}
</script>

<style>

</style>
