<template>
<view class="content">
    <u-row>
        <u-col :span="10">
            <u-sticky bgColor="#fff">
                <u-tabs :list="status_list" @change="update_status_filter"></u-tabs>
            </u-sticky>
        </u-col>
        <u-col :span="2">
            <u-button type="primary" text="新增" size="small" @click="create_diag = true"></u-button>
        </u-col>
    </u-row>
    <u-popup mode="top" :show="create_diag" @close="create_diag = false">
        <view>
            <u--form :labelWidth="150" :model="new_order" :rules="create_rules" ref="order_form">
                <u-form-item label="车号" prop="plate_number" borderBottom>
                    <u--input v-model="new_order.plate_number" border="none"></u--input>
                </u-form-item>
                <u-form-item label="挂车号" prop="back_plate_number" borderBottom>
                    <u--input v-model="new_order.back_plate_number" border="none"></u--input>
                </u-form-item>
                <u-form-item label="派车公司" prop="company_name" borderBottom>
                    <u--input v-model="new_order.company_name" border="none"></u--input>
                </u-form-item>
                <u-form-item label="司机姓名" prop="driver_name" borderBottom>
                    <u--input v-model="new_order.driver_name" border="none"></u--input>
                </u-form-item>
                <u-form-item label="司机电话" prop="driver_phone" borderBottom>
                    <u--input v-model="new_order.driver_phone" border="none"></u--input>
                </u-form-item>
                <u-form-item label="司机身份证号" prop="driver_id" borderBottom>
                    <u--input v-model="new_order.driver_id" border="none"></u--input>
                </u-form-item>
                <u-form-item label="物料名" prop="stuff_name" borderBottom>
                    <u--input v-model="new_order.stuff_name" border="none"></u--input>
                </u-form-item>
                <u-form-item label="连续派车截止日期" prop="continue_untile" borderBottom @click="show_continue_picker = true; hideKeyboard()">
                    <u--input v-model="new_order.continue_until" disabled disabledColor="#ffffff" placeholder="不填写即不连续派车" border="none"></u--input>
                    <u-icon slot="right" name="arrow-right"></u-icon>
                </u-form-item>
                <u-form-item label="进厂类型" prop="is_sale" borderBottom>
                    <u-radio-group v-model="sale_choose" placement="row">
                        <u-radio name="采购" label="采购"></u-radio>
                        <u-radio name="销售" label="销售"></u-radio>
                    </u-radio-group>
                </u-form-item>
                <!-- <u-form-item label="姓名" prop="plate_number" borderBottom>
                    <u--input v-model="new_order.plate_number" border="none"></u--input>
                </u-form-item>
                <u-form-item label="姓名" prop="plate_number" borderBottom>
                    <u--input v-model="new_order.plate_number" border="none"></u--input>
                </u-form-item> -->
                <u-button text="提交" type="success" @click="add_new_order"></u-button>
            </u--form>
        </view>
    </u-popup>
    <u-datetime-picker :show="show_continue_picker" mode="date" closeOnClickOverlay @confirm="confirm_continue_until" @cancel="clear_continue_until" @close="close_continue_until" :minDate="min_continue_date"></u-datetime-picker>
    <u-cell title="日期范围" :value="date_range" @click="show_date_picker = true"></u-cell>
    <u-calendar :show="show_date_picker" mode="range" @confirm="handle_confirm_date" :monthNum="48" allowSameDay :minDate="minDate" :maxDate="maxDate" @close="show_date_picker = false"></u-calendar>
    <u-search placeholder="输入车号过滤" v-model="search_key" :showAction="false" @change="load_more"></u-search>
    <u-list @scrolltolower="load_more" showScrollbar lowerThreshold="50">
        <u-list-item v-for="(item, index) in order_need_show" :key="index">
            <u-swipe-action>
                <u-swipe-action-item :options="options1" @click="dup_order(item)">
                    <u-cell :title="item.plate_number + '-' + item.back_plate_number" :label="item.driver_name + '-' + item.driver_phone" isLink :url="'/pages/detail?order_number=' + item.order_number">
                        <view slot="value" style="text-align:right">
                            <view>
                                {{item.stuff_name}}
                                {{item.is_sale?'皮':'毛'}}{{item.p_weight.toFixed(2)}}
                                {{item.is_sale?'毛':'皮'}}{{item.m_weight.toFixed(2)}}
                            </view>
                            <view>
                                {{item.company_name}}
                            </view>
                            <view>
                                左滑复制
                            </view>
                        </view>
                        <view slot="icon">
                            <view>
                                <u-icon v-if="item.status==100 && item.p_weight != 0 && item.m_weight != 0" name="checkbox-mark" color="green" size="28"></u-icon>
                                <u-icon v-else-if="item.status==100" name="man-delete" color="red" size="28"></u-icon>
                                <u-icon v-else-if="item.status==1" name="clock" color="blue" size="28"></u-icon>
                                <u-icon v-else name="hourglass-half-fill" color="yellow" size="28"></u-icon>
                            </view>
                            <view v-if="item.status == 100">
                                {{Math.abs(item.p_weight - item.m_weight).toFixed(2)}}
                            </view>
                        </view>
                    </u-cell>
                </u-swipe-action-item>
            </u-swipe-action>

        </u-list-item>
    </u-list>
</view>
</template>

<script>
import PinyinMatch from 'pinyin-match';
import moment from 'moment'
export default {
    data() {
        return {
            show_continue_picker: false,
            create_rules: {
                plate_number: {
                    type: 'string',
                    required: true,
                    message: '请输入车号',
                    trigger: 'blur',
                },
                driver_phone: {
                    type: 'string',
                    required: true,
                    message: '请输入司机电话',
                    trigger: 'blur',
                },
            },
            options1: [{
                text: '复制'
            }],
            create_diag: false,
            new_order: {
                back_plate_number: '',
                company_name: '',
                driver_id: '',
                driver_name: '',
                driver_phone: '',
                enter_weight: 0,
                is_sale: true,
                plate_number: '',
                stuff_from: '',
                stuff_name: '',
                continue_until: '',
            },
            all_orders: [],
            search_key: '',
            status_filter: 0,
            status_list: [{
                    name: '全部',
                    value: 0,
                },
                {
                    name: '未开始',
                    value: 1,
                    badge: {
                        value: 0
                    },
                },
                {
                    name: '正在进行',
                    value: 2,
                    badge: {
                        value: 0
                    },
                },
                {
                    name: '已完成',
                    value: 100,
                },
            ],
            is_loading: false,
            is_finished: false,
            date_begin: '',
            date_end: '',
            show_date_picker: false,
            min_continue_date: 0,
        }
    },
    computed: {
        sale_choose: {
            get: function () {
                var ret = '销售';
                if (this.new_order.is_sale == false) {
                    ret = '采购';
                }
                return ret;
            },
            set: function (val) {
                if (val == '采购')
                    this.new_order.is_sale = false;
                else if (val == '销售') {
                    this.new_order.is_sale = true;
                }
            }
        },
        maxDate: function () {
            var ret = '';
            var date = new Date();
            date.setDate(date.getDate() + 1);
            ret = date.toISOString().slice(0, 10);
            return ret;
        },
        minDate: function () {
            var ret = '';
            var date = new Date();
            date.setDate(date.getDate() - 60);
            ret = date.toISOString().slice(0, 10);
            return ret;
        },
        date_range: function () {
            return this.date_begin + ' - ' + this.date_end;
        },
        need_load_more: function () {
            return this.is_loading == false && this.is_finished == false;
        },
        order_need_show: function () {
            var vue_this = this;
            var ret = [];
            this.all_orders.forEach(item => {
                if (this.search_key.length == 0 || PinyinMatch.match(item.plate_number, vue_this.search_key)) {
                    ret.push(item);
                }
            });

            return ret;
        }
    },
    onPullDownRefresh() {
        this.all_orders = [];
        this.is_finished = false;
        this.load_more();
    },
    onLoad() {
        this.$send_req('/api/order/search', {}).then(res => {

        }).catch(err => {
            if (err) {
                uni.navigateTo({
                    url: '/pages/login?domain=' + uni.getStorageSync('domain'),
                })
            }
        });
        this.date_begin = new Date().toISOString().slice(0, 10);
        this.min_continue_date = new Date().getTime();
        this.date_end = new Date().toISOString().slice(0, 10);
        this.fetch_orders();
        this.fetch_order_count();
    },
    methods: {
        confirm_continue_until: function (e) {
            this.show_continue_picker = false;
            console.log(e.value);
            this.new_order.continue_until = moment(e.value).format('YYYY-MM-DD');
        },
        clear_continue_until: function () {
            this.new_order.continue_until = '';
            this.show_continue_picker = false;
        },
        close_continue_until: function () {
            this.show_continue_picker = false;
        },
        hideKeyboard() {
            uni.hideKeyboard()
        },
        dup_order: function (_order) {
            console.log(_order);
            this.new_order.company_name = _order.company_name;
            this.new_order.back_plate_number = _order.back_plate_number;
            this.new_order.driver_id = _order.driver_id;
            this.new_order.driver_name = _order.driver_name;
            this.new_order.driver_phone = _order.driver_phone;
            this.new_order.enter_weight = _order.enter_weight;
            this.new_order.is_sale = _order.is_sale;
            this.new_order.plate_number = _order.plate_number;
            this.new_order.stuff_from = _order.stuff_from;
            this.new_order.stuff_name = _order.stuff_name;
            this.create_diag = true;
        },
        add_new_order: function () {
            this.new_order.enter_weight = parseFloat(this.new_order.enter_weight);
            this.new_order.plate_number = this.new_order.plate_number.toUpperCase();
            this.new_order.back_plate_number = this.new_order.back_plate_number.toUpperCase();
            this.$refs.order_form.validate().then(res => {
                this.$send_req('/api/order/add', this.new_order).then(resp => {
                    if (resp) {
                        this.create_diag = false;
                        this.new_order = {
                            back_plate_number: '',
                            company_name: '',
                            driver_id: '',
                            driver_name: '',
                            driver_phone: '',
                            enter_weight: 0,
                            is_sale: true,
                            plate_number: '',
                            stuff_from: '',
                            stuff_name: '',
                            continue_until: '',
                        };
                        uni.startPullDownRefresh();
                    }
                });
            }).catch(errors => {
                uni.$u.toast('参数错误')
            })
        },
        fetch_order_count: function () {
            this.$send_req('/api/order/count', {
                status: 1,
            }).then(res => {
                this.status_list[1].badge.value = res;
                this.wait_status_count = res;
            });
            this.$send_req('/api/order/count', {
                status: 2,
            }).then(res => {
                this.status_list[2].badge.value = res;
            });

        },
        handle_confirm_date: function (date_array) {
            this.date_begin = date_array[0];
            this.date_end = date_array[date_array.length - 1]
            this.show_date_picker = false;
            this.all_orders = [];
            this.is_finished = false;
            this.load_more();
        },
        load_more: function () {
            if (this.need_load_more) {
                this.fetch_orders();
            }
            this.fetch_order_count();
        },
        update_status_filter: function (item) {
            this.status_filter = item.value;
            this.all_orders = [];
            this.is_finished = false;
            this.load_more();
        },
        fetch_orders: function () {
            this.is_loading = true;
            let req_begin_time = this.date_begin;
            let next_day = new Date(this.date_end);
            next_day.setDate(next_day.getDate() + 1);
            let req_end_time = next_day.toISOString().slice(0, 10);

            if (this.status_filter == 1) {
                req_begin_time = '';
                req_end_time = '';
            }
            let create_time_begin = '';
            let create_time_end = '';
            if (this.status_filter == 0) {
                create_time_begin = req_begin_time;
                create_time_end = req_end_time;
                req_begin_time = '';
                req_end_time = '';
            }
            this.$send_req('/api/order/search', {
                page_no: parseInt(this.all_orders.length / 20),
                status: this.status_filter,
                begin_time: req_begin_time,
                end_time: req_end_time,
                create_time_begin: create_time_begin,
                create_time_end: create_time_end,
            }).then(res => {
                res.forEach(item => {
                    this.all_orders.push(item);
                });
                if (res.length < 20) {
                    this.is_finished = true;
                }
            }).finally(() => {
                this.is_loading = false;
                uni.stopPullDownRefresh();
            })
        },
    }
}
</script>

<style>

</style>
