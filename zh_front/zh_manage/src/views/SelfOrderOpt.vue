<template>
<div class="self_order_opt_show">
    <van-nav-bar title="司机派车" right-text="刷新" @click-right="refresh" left-text="二维码" @click-left="show_qr = true" />
    <van-card v-for="(single_order, index) in all_self_orders" :key="index" :desc="single_order.driver_name" :title="single_order.main_vehicle_number">
        <template #price>
            {{single_order.driver_phone}}
        </template>
        <template #num>
            {{single_order.stuff_name}}
        </template>
        <template #tags>
            <van-tag plain type="danger">{{single_order.driver_id}}</van-tag>
        </template>
        <template #footer>
            <van-button size="small" type="danger" @click="cancel_self_order(single_order)">取消</van-button>
            <van-button size="small" type="primary" @click="confirm_self_order(single_order)">确认</van-button>
        </template>
    </van-card>
    <van-dialog v-model="show_qr" title="请司机扫描此码自助派车和排号" :showConfirmButton="false" closeOnClickOverlay>
        <div style="text-align: center;">
            <vue-qr :text="'http://' + domain_name + '/#/mobile/self_order?user=' + $store.state.user_info.name"></vue-qr>
            <div>
                {{$store.state.user_info.name}}
            </div>
        </div>
    </van-dialog>
</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import 'vant/lib/index.css';
import vueQr from 'vue-qr'
Vue.use(Vant);
export default {
    name: 'SelfOrderOpt',
    data: function () {
        return {
            show_qr: false,
            all_self_orders: [],
            domain_name: '',
        };
    },
    components: {
        vueQr
    },
    methods: {
        refresh: function () {
            this.init_self_order();
        },
        init_self_order: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_all_self_order", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.all_self_orders = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_self_orders, index, element);
                });
            });
        },
        confirm_self_order: function (_order) {
            var vue_this = this;
            var func = function (continue_order) {
                vue_this.$call_remote_process("vehicle_order_center", "confirm_driver_self_order", [vue_this.$cookies.get("zh_ssid"), _order.id, continue_order]).then(function (resp) {
                    if (resp) {
                        vue_this.init_self_order();
                    }
                });
            }
            vue_this.$dialog.confirm({
                title: '请指定派车方式?',
                confirmButtonText: '连续派车',
                cancelButtonText: '单次派车'
            }).then(function () {
                func(true);
            }).catch(function () {
                func(false);
            });

        },
        cancel_self_order: function (_order) {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "cancel_driver_self_order", [vue_this.$cookies.get("zh_ssid"), _order.id]).then(function (resp) {
                if (resp) {
                    vue_this.init_self_order();
                }
            });
        },
    },
    beforeMount: function () {
        var vue_this = this;
        this.init_self_order();
        vue_this.$call_remote_process("system_management", "get_domain_name", []).then(function (resp) {
            vue_this.domain_name = resp;
        });
    },
}
</script>

<style>

</style>
