<template>
<div class="field_opt_show">
    <van-nav-bar class="nav_bar_show" @click-left="$router.replace({name:'AllCameVehicle'})" title="装卸货确认">
        <template #left>
            <van-icon name="home-o" color="red" size="20">主页</van-icon>
        </template>
    </van-nav-bar>
    <van-cell :title="cur_order.basic_info.main_vehicle_number" :value="cur_order.basic_info.stuff_name" :label="cur_order.basic_info.source_dest_name"></van-cell>
    <van-cell :title="cur_order.basic_info.driver_name" :value="cur_order.basic_info.driver_phone" :label="cur_order.basic_info.company_name"></van-cell>
    <van-cell title="一次称重" :value="cur_order.basic_info.p_weight"></van-cell>
    <div v-if="cur_order.basic_info.status >= 4">
        <van-cell title="二次称重" :value="cur_order.basic_info.m_weight"></van-cell>
        <van-cell v-if="stuff_change > 0" title="净重（卸货）" :value="j_weight"></van-cell>
        <van-cell v-if="stuff_change < 0" title="净重（拉货）" :value="j_weight"></van-cell>
    </div>
    <div style="margin:15px;" v-if="cur_order.basic_info.status >= 1 && cur_order.basic_info.status != 100">
        <van-button v-if="!cur_order.confirmed" block type="primary" @click="confirm_deliver(true)">确认</van-button>
        <van-button v-else block type="danger" @click="confirm_deliver(false)">取消确认</van-button>
    </div>
    <div style="margin:15px;" v-if="cur_order.basic_info.status >= 4 && need_print_ticket">
        <van-button block type="info" @click="ticket_ditail_change_diag = true">打票</van-button>
    </div>
    <van-dialog v-model="ticket_ditail_change_diag" title="煤票打印" :showConfirmButton="false" closeOnClickOverlay>
        <van-form @submit="print_ticket">
            <van-field v-model="p_weight" type="number" name="皮重" label="皮重" placeholder="请输入皮重" :rules="[{ required: true, message: '请填写皮重' }]" />
            <van-field v-model="m_weight" type="number" name="毛重" label="毛重" placeholder="请输入毛重" :rules="[{ required: true, message: '请填写毛重' }]" />
            <van-field :value="(m_weight-p_weight).toFixed(2)" disabled name="净重" label="净重" />
            <van-field readonly clickable name="picker" :value="dest_company" label="运往公司" placeholder="点击选择公司" @click="show_company_Picker = true" />
            <van-popup v-model="show_company_Picker" position="bottom">
                <van-picker show-toolbar :columns="dest_company_pool" @confirm="on_company_confirm" @cancel="show_company_Picker = false" />
            </van-popup>
            <van-field readonly clickable name="picker" :value="address" label="运往地址" placeholder="点击选择地址" @click="showPicker = true" />
            <van-popup v-model="showPicker" position="bottom">
                <van-picker show-toolbar :columns="all_address" @confirm="onConfirm" @cancel="showPicker = false" />
            </van-popup>
            <div style="margin: 16px;">
                <van-button round block type="info" native-type="submit">提交</van-button>
            </div>
        </van-form>
    </van-dialog>

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
            ticket_base_data: {},
            dest_company_pool: [],
        };
    },
    computed: {
        need_print_ticket: function () {
            var ret = false;

            if (this.ticket_base_data && this.ticket_base_data.products) {
                this.ticket_base_data.products.forEach(element => {
                    if (element.Name == this.cur_order.basic_info.stuff_name) {
                        ret = true;
                    }
                });
            }

            return ret;
        },
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

            return ret;
        },
        stuff_change: function () {
            return this.cur_order.basic_info.p_weight - this.cur_order.basic_info.m_weight;
        },
    },
    methods: {
        on_company_confirm: function (_value) {
            this.dest_company = _value;
            this.show_company_Picker = false;
        },
        onConfirm: function (_value) {
            this.address = _value;
            this.showPicker = false;
        },
        print_ticket: function () {
            var vue_this = this;
            vue_this.p_weight = parseFloat(vue_this.p_weight);
            vue_this.m_weight = parseFloat(vue_this.m_weight);
            var print_req = {
                "main_vehicle_number": vue_this.cur_order.basic_info.main_vehicle_number,
                "p_weight": vue_this.p_weight.toFixed(2),
                "m_weight": vue_this.m_weight.toFixed(2),
                "axes": "6",
                "j_weight": (vue_this.m_weight - vue_this.p_weight).toFixed(2),
                "finish_date": vue_this.$make_time_string(new Date(), '-'),
                "stuff_name": vue_this.cur_order.basic_info.stuff_name,
                "company_name": vue_this.dest_company,
                "company_address": vue_this.address,
                "use_for": vue_this.cur_order.basic_info.use_for,
            };
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "finish '" + JSON.stringify(print_req) + "'"]).then(function (resp) {
                if (resp) {
                    vue_this.init_order();
                    vue_this.ticket_ditail_change_diag = false;
                }
            });
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
            });
        },
        init_base_data: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_ordos_ticket", "get -k basic_data"]).then(function (resp) {
                vue_this.ticket_base_data = JSON.parse(resp);
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
    },
    beforeMount: function () {
        this.init_order();
        this.init_base_data();
        this.init_dest_company_pool();
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}
</style>
