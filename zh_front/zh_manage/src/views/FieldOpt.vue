<template>
<div class="field_opt_show">
    <van-nav-bar class="nav_bar_show" @click-left="$router.replace({name:'AllCameVehicle'})" title="装卸详情">
        <template #left>
            <van-icon name="home-o" color="red" size="20">主页</van-icon>
        </template>
    </van-nav-bar>
    <van-divider>装卸货确认</van-divider>
    <van-cell :title="cur_order.basic_info.main_vehicle_number" :value="cur_order.basic_info.stuff_name" :label="cur_order.basic_info.source_dest_name"></van-cell>
    <van-cell :title="cur_order.basic_info.driver_name" :value="cur_order.basic_info.driver_phone" :label="cur_order.basic_info.company_name"></van-cell>
    <van-cell title="一次称重" :value="cur_order.basic_info.p_weight.toFixed(2)"></van-cell>
    <div v-if="cur_order.basic_info.status >= 4">
        <van-cell title="二次称重" :value="cur_order.basic_info.m_weight.toFixed(2)"></van-cell>
        <van-cell v-if="stuff_change > 0" title="净重（卸货）" :value="j_weight"></van-cell>
        <van-cell v-if="stuff_change < 0" title="净重（拉货）" :value="j_weight"></van-cell>
    </div>
    <div style="margin:15px;" v-if="cur_order.basic_info.status >= 1 && cur_order.basic_info.status != 100">
        <van-button v-if="!cur_order.confirmed" block type="primary" @click="confirm_deliver(true)">确认</van-button>
        <van-button v-else block type="danger" @click="confirm_deliver(false)">取消确认</van-button>
    </div>
    <div v-if="need_seal_no">
        <van-divider>铅封号</van-divider>
        <van-field v-model="seal_no" label="铅封号" placeholder="请输入铅封号" />
        <div style="margin:15px;" v-if="cur_order.basic_info.status != 100">
            <van-button block type="info" @click="set_seal_no">保存</van-button>
        </div>
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
            seal_no: '',
            need_seal_no: false,
            show_company_Picker: false,
            showPicker: false,
            ticket_ditail_change_diag: false,
            p_weight: 0,
            m_weight: 0,
            address: '',
            dest_company: '',
            cur_order: {
                confirmed: false,
                basic_info: {},
            },
            dest_company_pool: [],
        };
    },
    computed: {
        all_address: function () {
            var ret = [];

            if (this.ticket_base_data && this.ticket_base_data.commonlyDestinations) {
                this.ticket_base_data.commonlyDestinations.forEach(element => {
                    ret.push(element.DestinationName);
                });
            }

            return ret;
        },
        j_weight: function () {
            var ret = 0;
            ret = Math.abs(this.cur_order.basic_info.p_weight - this.cur_order.basic_info.m_weight);

            return ret.toFixed(2);
        },
        stuff_change: function () {
            return this.cur_order.basic_info.p_weight - this.cur_order.basic_info.m_weight;
        },
    },
    methods: {
        set_seal_no: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "set_seal_no", [vue_this.$cookies.get("zh_ssid"), this.cur_order.basic_info.order_number, this.seal_no]).then(function (resp) {
                if (resp) {
                    vue_this.init_order();
                }
            });
        },
        on_company_confirm: function (_value) {
            this.dest_company = _value;
            this.show_company_Picker = false;
        },
        onConfirm: function (_value) {
            this.address = _value;
            this.showPicker = false;
        },
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
                vue_this.p_weight = vue_this.cur_order.basic_info.p_weight;
                vue_this.m_weight = vue_this.cur_order.basic_info.m_weight;
                vue_this.address = vue_this.cur_order.basic_info.company_address;
                vue_this.dest_company = vue_this.cur_order.basic_info.source_dest_name;
                vue_this.seal_no = vue_this.cur_order.basic_info.seal_no;
            });
        },
        init_dest_company_pool: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", "get_all_source_dest", [false]).then(function (resp) {
                vue_this.dest_company_pool = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.dest_company_pool, index, element.name);
                });
            });
        },
        init_need_seal_no: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "need_seal_no", []).then(function (resp) {
                vue_this.need_seal_no = resp;
            });
        },
    },
    beforeMount: function () {
        this.init_order();
        this.init_dest_company_pool();
        this.init_need_seal_no();
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}
</style>
