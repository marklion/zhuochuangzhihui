<template>
<div class="driver_self_opt_show">
    <van-nav-bar title="自助操作" right-text="刷新" @click-right="refresh" />
    <van-dialog v-model="init_diag" title="请输入手机号查询" :showConfirmButton="false" closeOnClickOverlay>
        <van-form @submit="search_driver_phone">
            <van-field v-model="query_phone" label="司机电话" placeholder="司机电话" :rules="[{ required: true, message: '请填写司机电话' },
                {
                    pattern: /^(13[0-9]|14[01456879]|15[0-35-9]|16[2567]|17[0-8]|18[0-9]|19[0-35-9])\d{8}$/,
                    message: '请输入正确手机号',
                }]" />
            <div style="margin: 16px;">
                <van-row :gutter="10">
                    <van-col :span="12">
                        <van-button block type="info" native-type="submit">查询派车</van-button>
                    </van-col>
                    <van-col :span="12">
                        <van-button block type="warning" @click="search_ticket_by_phone" native-type="button">查看磅单</van-button>
                    </van-col>
                </van-row>
            </div>
        </van-form>
    </van-dialog>
    <van-tabs v-model="active">
        <van-tab title="自助派车">
            <van-notice-bar left-icon="volume-o" text="提交后请等待货主审批和厂区审批" />
            <van-form @submit="create_self_order">
                <van-field :disabled="$route.query.hasOwnProperty('user')" v-model="new_self_order.belong_user_name" label="公司" placeholder="公司" :rules="[{ required: true, message: '请填写公司名' }]" />
                <van-field v-model="new_self_order.main_vehicle_number" label="车牌号" placeholder="车牌号" :rules="[{ required: true, message: '请填写车牌号' },{
                        pattern: /^(京[A-HJ-NPQY]|沪[A-HJ-N]|津[A-HJ-NPQR]|渝[A-DFGHN]|冀[A-HJRST]|晋[A-FHJ-M]|蒙[A-HJKLM]|辽[A-HJ-NP]|吉[A-HJK]|黑[A-HJ-NPR]|苏[A-HJ-N]|浙[A-HJKL]|皖[A-HJ-NP-S]|闽[A-HJK]|赣[A-HJKLMS]|鲁[A-HJ-NP-SUVWY]|豫[A-HJ-NP-SU]|鄂[A-HJ-NP-S]|湘[A-HJ-NSU]|粤[A-HJ-NP-Y]|桂[A-HJ-NPR]|琼[A-F]|川[A-HJ-MQ-Z]|贵[A-HJ]|云[AC-HJ-NP-SV]|藏[A-HJ]|陕[A-HJKV]|甘[A-HJ-NP]|青[A-H]|宁[A-E]|新[A-HJ-NP-S])([0-9A-HJ-NP-Z]{4}[0-9A-HJ-NP-Z挂试]|[0-9]{4}学|[A-D0-9][0-9]{3}警|[DF][0-9A-HJ-NP-Z][0-9]{4}|[0-9]{5}[DF])$|^WJ[京沪津渝冀晋蒙辽吉黑苏浙皖闽赣鲁豫鄂湘粤桂琼川贵云藏陕甘青宁新]?[0-9]{4}[0-9JBXTHSD]$|^(V[A-GKMORTV]|K[A-HJ-NORUZ]|H[A-GLOR]|[BCGJLNS][A-DKMNORVY]|G[JS])[0-9]{5}$|^[0-9]{6}使$|^([沪粤川渝辽云桂鄂湘陕藏黑]A|闽D|鲁B|蒙[AEH])[0-9]{4}领$|^粤Z[0-9A-HJ-NP-Z][0-9]{3}[港澳]$/i,
                        message: '请输入正确的车牌',
                }]" />
                <van-field v-model="new_self_order.driver_name" label="司机姓名" placeholder="司机姓名" :rules="[{ required: true, message: '请填写司机姓名' }]" />
                <van-field v-model="new_self_order.driver_phone" label="司机电话" placeholder="司机电话" :rules="[{ required: true, message: '请填写司机电话' },
                {
                    pattern: /^(13[0-9]|14[01456879]|15[0-35-9]|16[2567]|17[0-8]|18[0-9]|19[0-35-9])\d{8}$/,
                    message: '请输入正确手机号',
                }]" />
                <van-field v-model="new_self_order.driver_id" label="司机身份证号" placeholder="司机身份证" :rules="[{ required: true, message: '请填写司机身份证' },
                {
                    pattern: /^[1-9]\d{5}(18|19|20|(3\d))\d{2}((0[1-9])|(1[0-2]))(([0-2][1-9])|10|20|30|31)\d{3}[0-9Xx]$/,
                    message: '请输入正确身份证号',
                }]" />
                <item-for-select label="运输货物" :related_contract="new_self_order.belong_user_name" v-model="new_self_order.stuff_name" :rules="[{required:true, message:'请指定货物'}]" search_key="stuff_name"></item-for-select>
                <div style="margin: 16px;">
                    <van-button round block type="info" native-type="submit">提交</van-button>
                </div>
            </van-form>
        </van-tab>
        <van-tab title="派车查询">
            <van-search v-model="query_phone" show-action label="司机电话" placeholder="请输入司机电话查询" @search="get_self_order">
                <template #action>
                    <div @click="get_self_order">搜索</div>
                </template>
            </van-search>
            <van-cell-group inset title="未审批的派车单" v-if="queried_record.main_vehicle_number.length > 0">
                <van-cell title="车牌号" :value="queried_record.main_vehicle_number" />
                <van-cell title="司机姓名" :value="queried_record.driver_name" />
                <van-cell title="司机电话" :value="queried_record.driver_phone" />
                <van-cell title="司机身份证" :value="queried_record.driver_id" />
                <van-cell title="拉运公司" :value="queried_record.belong_user_name" />
                <van-cell title="货物名称" :value="queried_record.stuff_name" />
                <div style="margin: 16px;">
                    <van-button round block type="info" @click="search_driver_phone">刷新</van-button>
                </div>
                <div style="margin: 16px;">
                    <van-button round block type="danger" @click="cancel_self_order">取消</van-button>
                </div>
            </van-cell-group>
            <van-empty description="搜索货主未批准的自助派车单" v-else />
        </van-tab>
        <van-tab title="排号">
            <van-search v-model="query_phone" label="司机电话" placeholder="请输入司机电话查询" show-action @search="check_in_phone = query_phone">
                <template #action>
                    <div @click="check_in_phone = query_phone">搜索</div>
                </template>
            </van-search>
            <check-in v-if="check_in_phone" :driver_phone="check_in_phone"></check-in>
            <van-empty v-else description="无派车记录,请输入正确司机手机号搜索">
            </van-empty>
        </van-tab>
        <van-tab title="历史磅单">
            <van-search v-model="query_phone" label="司机电话" placeholder="请输入司机电话查询" show-action @search="search_ticket_by_phone">
                <template #action>
                    <div @click="search_ticket_by_phone">搜索</div>
                </template>
            </van-search>
            <van-cell v-for="(single_vehicle, index) in history_ticket" :key="index" :title="single_vehicle.basic_info.main_vehicle_number" :value="single_vehicle.m_time" :label="single_vehicle.basic_info.stuff_name" is-link :to="{name:'TicketPage', params:{order_number:single_vehicle.basic_info.order_number}}"></van-cell>
        </van-tab>
    </van-tabs>
</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import 'vant/lib/index.css';
import ItemForSelect from "../components/ItemForSelect.vue"
import CheckIn from "../components/CheckIn.vue"
Vue.use(Vant);

export default {
    name: 'DriverSelfOpt',
    components: {
        "item-for-select": ItemForSelect,
        "check-in": CheckIn,
    },
    data: function () {
        return {
            init_diag: true,
            check_in_phone: '',
            active: 0,
            new_self_order: {
                driver_id: '',
                driver_phone: '',
                driver_name: '',
                main_vehicle_number: '',
                stuff_name: '',
                belong_user_name: '',
            },
            query_phone: '',
            queried_record: {
                driver_id: '',
                driver_phone: '',
                driver_name: '',
                main_vehicle_number: '',
                stuff_name: '',
                belong_user_name: '',
            },
            history_ticket: [],
        };
    },
    methods: {
        cancel_self_order: function () {
            var vue_this = this;
            vue_this.$dialog.confirm({
                title: '取消确认',
                message: '确认取消自助派车单吗？',
                cancelButtonText: '再等等'
            }).then(function () {
                vue_this.$call_remote_process("vehicle_order_center", "cancel_driver_self_order", ["", vue_this.queried_record.id]).then(function (resp) {
                    if (resp) {
                        vue_this.refresh();
                    }
                });
            });
        },
        search_driver_phone: function () {
            var vue_this = this;
            vue_this.$call_remote_process_no_toast("vehicle_order_center", "driver_get_order", [vue_this.query_phone]).then(function (resp) {
                if (resp) {
                    vue_this.active = 2;
                    vue_this.check_in_phone = vue_this.query_phone;
                }
            }).catch(function () {
                vue_this.$call_remote_process_no_toast("vehicle_order_center", "get_self_order_by_phone", [vue_this.query_phone]).then(function (resp) {
                    vue_this.queried_record = resp;
                    vue_this.active = 1;
                    vue_this.$dialog.alert({
                        title: "等待审批",
                        message: "已提交自助派车，请联系货主审批,\n货主审批通过后方可请刷新页面操作排号"
                    });
                }).catch(function () {
                    vue_this.$dialog.alert({
                        title: '未找到派车单',
                        message: '请完善信息后提交派车单'
                    });
                    vue_this.$call_remote_process_no_toast("vehicle_order_center", "get_driver_opt_history", [vue_this.query_phone]).then(function (resp) {
                        vue_this.new_self_order.main_vehicle_number = resp.main_vehicle_number;
                        vue_this.new_self_order.driver_id = resp.driver_id;
                        vue_this.new_self_order.driver_phone = vue_this.query_phone;
                        vue_this.new_self_order.driver_name = resp.driver_name;
                    });
                    vue_this.active = 0;
                });
            }).finally(function () {
                vue_this.init_diag = false;
            });
        },
        create_self_order: function () {
            var vue_this = this;
            vue_this.new_self_order.main_vehicle_number = vue_this.new_self_order.main_vehicle_number.toUpperCase()
            vue_this.$call_remote_process("vehicle_order_center", "create_driver_self_order", [vue_this.new_self_order]).then(function (resp) {
                if (resp) {
                    vue_this.search_driver_phone();
                }
            })
        },
        get_self_order: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_self_order_by_phone", [vue_this.query_phone]).then(function (resp) {
                vue_this.queried_record = resp;
            });
        },
        refresh: function () {
            window.location.reload();
        },
        search_ticket_by_phone: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "driver_get_last_30_order_number", [vue_this.query_phone]).then(function (resp) {
                vue_this.history_ticket = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.history_ticket, index, element);
                });
                vue_this.init_diag = false;
                vue_this.active = 3;
            });
        },
    },
    beforeMount: function () {
        if (this.$route.query.user) {
            this.new_self_order.belong_user_name = this.$route.query.user;
        }
    },
}
</script>

<style>

</style>
