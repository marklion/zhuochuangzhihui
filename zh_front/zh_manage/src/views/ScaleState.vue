<template>
<div class="scale_state_show">
    <van-nav-bar title="设备状态" @click-right="init_all_device" right-text="刷新" />
    <div v-for="(single_device, index) in all_device" :key="index">
        <van-divider />
        <device-opt :device_status="single_device" @refresh="init_all_device"></device-opt>
    </div>
</div>
</template>

<script>
import Vue from 'vue'

import vueFlvPlayer from 'vue-flv-player'
import DeviceOpt from "../components/DeviceOpt.vue"
Vue.use(vueFlvPlayer)
export default {
    name: 'ScaleState',
    data: function () {
        return {
            active: 0,
            all_video_param: [],
            push_order_number: '',
            all_device: [],
        };
    },
    components: {
        'device-opt': DeviceOpt,
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
        init_all_device: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_all_device", []).then(function (resp) {
                vue_this.all_device = [];
                resp.forEach((item, index) => {
                    vue_this.$set(vue_this.all_device, index, item);
                });
            });
        },
    },
    beforeMount: function () {
        this.init_video_path();
        this.init_all_device();
    },
}
</script>

<style>

</style>
