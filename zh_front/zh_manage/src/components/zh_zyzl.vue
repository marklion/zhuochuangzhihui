<template>
<div class="zh_zyzl_show">
    <span>远端地址</span>
    <el-input v-model="remote_url"></el-input>
    <span>令牌</span>
    <el-input v-model="token"></el-input>
    <el-button type="primary" @click="save_param" size="small">保存</el-button>
</div>
</template>

<script>
export default {
    name: "zh_zyzl",
    data: function () {
        return {
            remote_url:'',
            token:'',
        };
    },
    methods: {
        init_data: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyzl', 'get -k zyzl_url']).then(function (resp) {
                if (resp) {
                    vue_this.remote_url = resp;
                }
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyzl', 'get -k token']).then(function (resp) {
                if (resp) {
                    vue_this.token= resp;
                }
            });
        },
        save_param: async function () {
            var vue_this = this;
            try {
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyzl', 'set -k zyzl_url ' + vue_this.remote_url]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyzl', 'set -k token ' + vue_this.token]);
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
