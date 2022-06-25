<template>
<div class="zh_meiyitong_show">
    <span>监管平台地址</span>
    <el-input v-model="remote_url"></el-input>
    <span>访问凭证</span>
    <el-input v-model="access_code"></el-input>
    <span>UKEY</span>
    <el-input v-model="ukey"></el-input>
    <span>GPS</span>
    <el-input v-model="gps"></el-input>
    <el-button size="small" type="primary" @click="set_config_data">保存</el-button>
    <el-button size="small" type="success" @click="sync_dest_addr">同步拉运目的地</el-button>
</div>
</template>

<script>
export default {
    name: "zh_meiyitong",
    data: function () {
        return {
            remote_url: '',
            access_code: "",
            ukey: "",
            gps: "",
        };
    },
    methods: {
        init_all_data: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k access_code"]).then(function (resp) {
                vue_this.access_code = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k remote_url"]).then(function (resp) {
                vue_this.remote_url = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k ukey"]).then(function (resp) {
                vue_this.ukey = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k gps"]).then(function (resp) {
                vue_this.gps = resp;
            });
        },
        set_config_data: async function () {
            var vue_this = this;
            try {
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "set -k access_code " + vue_this.access_code]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "set -k remote_url " + vue_this.remote_url]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "set -k ukey " + vue_this.ukey]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "set -k gps " + vue_this.gps]);
                vue_this.init_all_data();
            } catch (error) {
                console.log(error);
            }
        },
        sync_dest_addr: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "fetch_dest"]).then(function (resp) {
                var all_dest = JSON.parse(resp);
                all_dest.locations.forEach(element => {
                    vue_this.$call_remote_process("stuff_management", "add_source_dest", [vue_this.$cookies.get("zh_ssid"), element.Name, false, element.ID]);
                });
            });
        }
    },
    beforeMount: function () {
        this.init_all_data();
    }
}
</script>

<style>

</style>
