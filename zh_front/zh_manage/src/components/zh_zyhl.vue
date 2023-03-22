<template>
<div class="zh_zyhl_show">
    <span>远端地址</span>
    <el-input v-model="zy_url"></el-input>
    <span>appid</span>
    <el-input v-model="appid"></el-input>
    <span>secret</span>
    <el-input v-model="secret"></el-input>
    <el-switch v-model="need_ystd" active-text="顺延昨日" inactive-text="关闭昨日">
    </el-switch>

    <el-button type="primary" @click="save_param" size="small">保存</el-button>

    <el-divider content-position="left">磅单导出</el-divider>
    <div>
        <span>日期范围</span>
        <el-date-picker v-model="date_range" type="daterange" align="right" unlink-panels range-separator="至" start-placeholder="开始日期" end-placeholder="结束日期" :picker-options="pickerOptions">
        </el-date-picker>
    </div>
    <span>承运公司</span>
    <el-input v-model="trans_company"></el-input>
    <el-button type="primary" @click="export_ticket" size="small">导出</el-button>
    <span v-if="'正在下载' == download_status">{{download_status}}</span>
    <a v-else :href="download_status">已完成，点击下载</a>
</div>
</template>

<script>
export default {
    name: "zh_zyhl",
    data: function () {
        return {
            download_status: '',
            zy_url: '',
            appid: '',
            secret: '',
            need_ystd: false,
            begin_date: new Date(new Date().setDate(new Date().getDate() - 7)),
            end_date: new Date(),
            trans_company: '',
            pickerOptions: {
                shortcuts: [{
                    text: '最近一周',
                    onClick(picker) {
                        const end = new Date();
                        const start = new Date();
                        start.setTime(start.getTime() - 3600 * 1000 * 24 * 7);
                        picker.$emit('pick', [start, end]);
                    }
                }, {
                    text: '最近一个月',
                    onClick(picker) {
                        const end = new Date();
                        const start = new Date();
                        start.setTime(start.getTime() - 3600 * 1000 * 24 * 30);
                        picker.$emit('pick', [start, end]);
                    }
                }, {
                    text: '最近三个月',
                    onClick(picker) {
                        const end = new Date();
                        const start = new Date();
                        start.setTime(start.getTime() - 3600 * 1000 * 24 * 90);
                        picker.$emit('pick', [start, end]);
                    }
                }]
            },
        };
    },
    computed: {
        date_range: {
            get: function () {
                var ret = [];
                ret.push(this.begin_date);
                ret.push(this.end_date);
                return ret;
            },
            set: function (_new_value) {
                this.begin_date = _new_value[0];
                this.end_date = _new_value[1];
            }
        },
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
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'get -k need_ystd']).then(function (resp) {
                if ("true" == resp) {
                    vue_this.need_ystd = true;
                } else {
                    vue_this.need_ystd = false;
                }
            });

            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'get -k download_status']).then(function (resp) {
                vue_this.download_status = resp;
            });
        },
        save_param: async function () {
            var vue_this = this;
            try {
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'set -k zyhl_url ' + vue_this.zy_url]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'set -k appid ' + vue_this.appid]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'set -k secret ' + vue_this.secret]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'set -k need_ystd ' + (vue_this.need_ystd ? 'true' : 'false')]);
            } catch (error) {
                console.log(error);
            }
        },
        export_ticket: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), 'zh_zyhl', 'download_ticket ' + vue_this.$make_time_string(vue_this.begin_date, '-').substr(0, 10) + ' ' + vue_this.$make_time_string(vue_this.end_date, '-').substr(0, 10) + ' ' + vue_this.trans_company]);
            setTimeout(() => {
                vue_this.init_param();
            }, 1);
        },
    },
    beforeMount: function () {
        this.init_param();
    }
}
</script>

<style>

</style>
