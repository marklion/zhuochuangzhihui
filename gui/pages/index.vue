<template>
<view class="content">
    <u-sticky bgColor="#fff">
        <u-tabs :list="status_list" @change="update_status_filter"></u-tabs>
    </u-sticky>
    <u-cell title="日期范围" :value="date_range" @click="show_date_picker = true"></u-cell>
    <u-calendar :show="show_date_picker" mode="range" @confirm="handle_confirm_date" :monthNum="48" allowSameDay :minDate="minDate" :maxDate="maxDate" @close="show_date_picker = false"></u-calendar>
    <u-search placeholder="输入车号过滤" v-model="search_key" :showAction="false" @change="load_more"></u-search>
    <u-list @scrolltolower="load_more" showScrollbar lowerThreshold="50">
        <u-list-item v-for="(item, index) in order_need_show" :key="index">
            <u-cell :title="item.plate_number + '-' + item.back_plate_number" :label="item.driver_name + '-' + item.driver_phone">
                <view slot="value" style="text-align:right">
                    <view>
                        {{item.stuff_name}}
                        皮{{item.p_weight.toFixed(2)}}
                        毛{{item.m_weight.toFixed(2)}}
                    </view>
                    <view>
                        {{item.company_name}}
                    </view>
                </view>
                <view slot="icon">
                    <view>
                        <u-icon v-if="item.status==100" name="checkbox-mark" color="green" size="28"></u-icon>
                        <u-icon v-else-if="item.status==1" name="clock" color="red" size="28"></u-icon>
                        <u-icon v-else name="hourglass-half-fill" color="yellow" size="28"></u-icon>
                    </view>
                    <view v-if="item.status == 100">
                        {{Math.abs(item.p_weight - item.m_weight).toFixed(2)}}
                    </view>
                </view>
            </u-cell>
        </u-list-item>
    </u-list>
</view>
</template>

<script>
import PinyinMatch from 'pinyin-match';
export default {
    data() {
        return {
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
        }
    },
    computed: {
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
        this.date_end = new Date().toISOString().slice(0, 10);
        this.fetch_orders();
        this.fetch_order_count();
    },
    methods: {
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
            if (this.need_load_more && this.order_need_show.length <= 20) {
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
            this.$send_req('/api/order/search', {
                page_no: parseInt(this.all_orders.length / 20),
                status: this.status_filter,
                begin_time: req_begin_time,
                end_time: req_end_time,
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
