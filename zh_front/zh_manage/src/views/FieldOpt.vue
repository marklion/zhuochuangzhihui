<template>
<div class="field_opt_show">
    <van-nav-bar class="nav_bar_show" title="装卸货确认" />
    <van-cell :title="cur_order.basic_info.main_vehicle_number" :label="cur_order.basic_info.behind_vehicle_number" :value="cur_order.basic_info.stuff_name"></van-cell>
    <van-cell :title="cur_order.basic_info.driver_name" :value="cur_order.basic_info.driver_phone" :label="cur_order.basic_info.company_name"></van-cell>
    <div style="margin:15px;">
        <van-button v-if="!cur_order.confirmed" block type="primary" @click="confirm_deliver(true)">确认</van-button>
        <van-button v-else block type="danger" @click="confirm_deliver(false)">取消确认</van-button>
    </div>
</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import 'vant/lib/index.css';
Vue.use(Vant);

export default {
    name: 'FieldOpt',
    data: function () {
        return {
            cur_order: {
                confirmed: false,
                basic_info:{},
            },
        };
    },
    methods: {
        confirm_deliver: function (_confirm) {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "confirm_order_deliver", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_order.basic_info.order_number, _confirm]).then(function (resp) {
                if (resp) {
                    window.location.reload();
                }
            });
        },
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("vehicle_order_center", "get_order_detail", [vue_this.$cookies.get("zh_ssid"), vue_this.$route.params.order_no]).then(function (resp) {
            vue_this.cur_order = resp;
            vue_this.$set(vue_this.cur_order, "basic_info", resp.basic_info);
        });
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}
</style>
