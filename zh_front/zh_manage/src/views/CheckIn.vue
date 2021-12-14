<template>
<div class="check_in_show">
    <van-nav-bar class="nav_bar_show" title="运输信息" />
    <van-cell :title="cur_vehicle.basic_info.main_vehicle_number" :label="cur_vehicle.basic_info.company_name" :value="cur_vehicle.basic_info.stuff_name"></van-cell>
    <van-cell :title="cur_vehicle.basic_info.driver_name" :value="cur_vehicle.basic_info.driver_phone" :label="cur_vehicle.basic_info.driver_id"></van-cell>
    <van-cell title="当前状态" :value="status" center>
        <template #right-icon>
            <van-button size="small" type="info" icon="replay" @click="init_vehicle">刷新</van-button>
        </template>
    </van-cell>
    <vue-qr :text="cur_url()" :size="200"></vue-qr>
    <div style="margin:15px;">
        <van-button v-if="!cur_vehicle.registered" block type="info" @click="driver_check_in(false)">排号</van-button>
        <van-button v-else block type="danger" @click="driver_check_in(true)">取消排号</van-button>
    </div>
</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import 'vant/lib/index.css';
import vueQr from 'vue-qr'
Vue.use(Vant);
export default {
    name: 'CheckIn',
    components: {
        vueQr
    },
    data: function () {
        return {
            cur_vehicle: {
                basic_info: {},
                confirmed: false,
                registered: false,
                has_called: false,
            },
            cur_url: function () {
                return document.location.href;
            },
        };
    },
    computed: {
        status: function () {
            var ret = "未排号";
            if (this.cur_vehicle.registered) {
                ret = '已排号, 等待叫号';
            }
            if (this.cur_vehicle.has_called) {
                ret = '已叫号, 可以进厂';
            }

            return ret;
        },
    },
    methods: {
        driver_check_in: function (_cancel) {
            var vue_this = this;
            if (!_cancel) {
                vue_this.$call_remote_process("vehicle_order_center", "driver_check_in", [parseInt(vue_this.cur_vehicle.basic_info.id), _cancel]).then(function (resp) {
                    if (resp) {
                        vue_this.init_vehicle();
                    }
                });
            } else {
                vue_this.$dialog.confirm({
                    title: "取消确认",
                    message: "确认取消排号吗？"
                }).then(function () {
                    vue_this.$call_remote_process("vehicle_order_center", "driver_check_in", [parseInt(vue_this.cur_vehicle.basic_info.id), _cancel]).then(function (resp) {
                        if (resp) {
                            vue_this.init_vehicle();
                        }
                    });
                });
            }

        },
        init_vehicle() {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "driver_get_order", [vue_this.$route.params.order_no]).then(function (resp) {
                vue_this.cur_vehicle = resp;
                vue_this.$set(vue_this.cur_vehicle, 'basic_info', resp.basic_info);
            });
        },
    },
    beforeMount: function () {
        this.init_vehicle();
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}
</style>
