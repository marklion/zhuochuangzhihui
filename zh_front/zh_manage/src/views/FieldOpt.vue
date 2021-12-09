<template>
<div class="field_opt_show">
    <van-nav-bar class="nav_bar_show" @click-left="$router.replace({name:'AllCameVehicle'})" title="装卸货确认">
        <template #left>
            <van-icon name="home-o" color="red" size="20">主页</van-icon>
        </template>
    </van-nav-bar>
    <van-cell :title="cur_order.basic_info.main_vehicle_number" :label="cur_order.basic_info.behind_vehicle_number" :value="cur_order.basic_info.stuff_name"></van-cell>
    <van-cell :title="cur_order.basic_info.driver_name" :value="cur_order.basic_info.driver_phone" :label="cur_order.basic_info.company_name"></van-cell>
    <van-cell title="一次称重" :value="cur_order.basic_info.p_weight"></van-cell>
    <div v-if="cur_order.basic_info.status >= 4">
        <van-cell title="二次称重" :value="cur_order.basic_info.m_weight"></van-cell>
        <van-cell v-if="stuff_change > 0" title="净重（卸货）" :value="j_weight"></van-cell>
        <van-cell v-if="stuff_change < 0" title="净重（拉货）" :value="j_weight"></van-cell>
    </div>
    <div style="margin:15px;" v-if="cur_order.basic_info.status == 3">
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
                basic_info: {},
            },
        };
    },
    computed: {
        j_weight: function () {
            var ret = 0;
            ret = Math.abs(this.cur_order.basic_info.p_weight - this.cur_order.basic_info.m_weight);

            return ret;
        },
        stuff_change: function () {
            return this.cur_order.basic_info.p_weight - this.cur_order.basic_info.m_weight;
        },
    },
    methods: {
        confirm_deliver: function (_confirm) {
            var vue_this = this;
            if (!_confirm) {
                vue_this.$dialog.confirm({
                    title: "取消确认",
                    message: "取消装卸货确认吗"
                }).then(function () {
                    vue_this.$call_remote_process("vehicle_order_center", "confirm_order_deliver", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_order.basic_info.order_number, _confirm]).then(function (resp) {
                        if (resp) {
                            vue_this.init_order();
                        }
                    });
                });
            } else {
                vue_this.$call_remote_process("vehicle_order_center", "confirm_order_deliver", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_order.basic_info.order_number, _confirm]).then(function (resp) {
                    if (resp) {
                        vue_this.init_order();
                    }
                });
            }

        },
        init_order: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_order_detail", [vue_this.$cookies.get("zh_ssid"), vue_this.$route.params.order_no]).then(function (resp) {
                vue_this.cur_order = resp;
                vue_this.$set(vue_this.cur_order, "basic_info", resp.basic_info);
            });
        },
    },
    beforeMount: function () {
        this.init_order();
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}
</style>
