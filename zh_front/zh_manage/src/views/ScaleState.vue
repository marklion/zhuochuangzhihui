<template>
<div class="scale_state_show">

    <van-nav-bar title="设备状态" @click-right="init_scale_state" right-text="刷新" />

    <van-tabs v-model="active">
        <van-tab title="称重状态">
            <van-cell v-for="(single_scale, index) in scale_state" :key="index" :title="single_scale.name" :label="single_scale.cur_status" :value="single_scale.weight_pip">
                <template #right-icon>
                    <van-button type="danger" size="small" @click="reset_scale(single_scale.name)">重置</van-button>
                </template>
            </van-cell>
        </van-tab>
        <van-tab title="监控">
            <van-dropdown-menu>
                <van-dropdown-item :options="all_video_param" @change="set_video_path" />
            </van-dropdown-menu>
            <vue-flv-player :autoplay="true" :controls="true" ref="myPlayer" source="/live?app=live&stream=home" />
        </van-tab>
    </van-tabs>
</div>
</template>

<script>
import Vue from 'vue'

import vueFlvPlayer from 'vue-flv-player'
Vue.use(vueFlvPlayer)
export default {
    name: 'ScaleState',
    data: function () {
        return {
            active: 0,
            scale_state: [],
            all_video_param: [],
        };
    },
    methods: {
        set_video_path: function (_path) {
            var vue_this = this;
            vue_this.$call_remote_process("open_api", "set_video_path", [vue_this.$cookies.get("zh_ssid"), _path]);
        },
        init_video_path: function () {
            var vue_this = this;
            vue_this.$call_remote_process("open_api", "get_all_video_path", []).then(function (resp) {
                vue_this.all_video_param = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_video_param, index, {
                        text: element.name,
                        value: element.path
                    });
                });
            });
        },
        init_scale_state: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_scale_state", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.scale_state = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.scale_state, index, element);
                });
            });
        },
        reset_scale: function (_scale_name) {
            var vue_this = this;
            vue_this.$dialog.confirm({
                message: '确定要重置' + _scale_name + '的状态吗？',
                title: '提示'
            }).then(() => {
                vue_this.$call_remote_process("system_management", "reset_scale_state", [vue_this.$cookies.get("zh_ssid"), _scale_name]).then(function () {
                    vue_this.init_scale_state();
                });
            });
        },
    },
    beforeMount: function () {
        this.init_scale_state();
        this.init_video_path();
    },
}
</script>

<style>

</style>
