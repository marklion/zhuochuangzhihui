<template>
<div class="all_came_vehicle_show">
    <van-nav-bar class="nav_bar_show" title="现场车辆管理" @click-right="init_all_vehicle" right-text="刷新" />
    <van-dropdown-menu>
        <van-dropdown-item v-model="company_filter" :options="company_options" />
        <van-dropdown-item v-model="stuff_filter" :options="stuff_options" />
    </van-dropdown-menu>
    <van-search v-model="search_key" placeholder="车牌号过滤" />
    <div class="single_vehicle_show" v-for="(single_vehicle, index) in vehicle_need_show" :key="index">
        <van-cell :title="single_vehicle.basic_info.company_name" :label="single_vehicle.basic_info.stuff_name" :value="single_vehicle.basic_info.main_vehicle_number" center>
        </van-cell>
        <van-cell title="司机" :value="single_vehicle.basic_info.driver_name" :label="single_vehicle.basic_info.driver_phone"></van-cell>
        <van-cell title="附件" v-if="single_vehicle.basic_info.attachment">
            <van-image width="100" height="100" :src="$remote_file_url + single_vehicle.basic_info.attachment" @click="preview_attachment(single_vehicle.basic_info.attachment)" />
        </van-cell>
        <van-cell title="叫号状态" :value="single_vehicle.has_called?'已叫号':'未叫号'" center>
            <template #right-icon>
                <div style="margin-left:10px;">
                    <van-button v-if="!single_vehicle.has_called" size="small" type="info" @click="call_vehicle(single_vehicle, false)">叫号</van-button>
                    <van-button v-else size="small" type="danger" @click="call_vehicle(single_vehicle, true)">取消叫号</van-button>
                </div>
            </template>
        </van-cell>
        <van-cell v-if="single_vehicle.has_called" title="详细信息" is-link :to="{name:'FieldOpt', params:{order_no:single_vehicle.basic_info.order_number}}"></van-cell>
    </div>

</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import 'vant/lib/index.css';
Vue.use(Vant);
import {
    ImagePreview
} from 'vant';
import PinyinMatch from "pinyin-match"
export default {
    name: 'AllCameVehicle',
    data: function () {
        return {
            search_key: '',
            all_vehicle: [],
            company_filter: '全部',
            stuff_filter: '全部',
            company_options: [{
                text: '全部公司',
                value: '全部'
            }],
            stuff_options: [{
                text: '全部物料',
                value: '全部'
            }],
        };
    },
    computed: {
        vehicle_need_show: function () {
            var ret = [];
            var vue_this = this;
            this.all_vehicle.forEach(element => {
                if (vue_this.company_filter == '全部') {
                    ret.push(element);
                } else if (vue_this.company_filter == element.basic_info.company_name) {
                    ret.push(element);
                }
            });
            var tmp_ret = ret;
            ret = [];
            tmp_ret.forEach(element => {
                if (vue_this.stuff_filter == '全部') {
                    ret.push(element);
                } else if (vue_this.stuff_filter == element.basic_info.stuff_name) {
                    ret.push(element);
                }
            });
            tmp_ret = ret;
            ret = [];
            tmp_ret.forEach(element => {
                if (vue_this.search_key.length == 0 || PinyinMatch.match(element.basic_info.main_vehicle_number, vue_this.search_key)) {
                    ret.push(element);
                }
            });

            return ret;
        },
    },
    methods: {
        preview_attachment: function (_attachment) {
            ImagePreview({
                images: [this.$remote_file_url + _attachment],
                closeable: true,
            });
        },
        call_vehicle: function (_vehicle, _is_cancel) {
            var vue_this = this;
            if (!_is_cancel) {
                vue_this.$call_remote_process("vehicle_order_center", "call_vehicle", [vue_this.$cookies.get("zh_ssid"), _vehicle.basic_info.id, _is_cancel]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_vehicle();
                    }
                });
            } else {
                vue_this.$dialog.confirm({
                    title: '取消确认',
                    message: '确认取消叫号吗？'
                }).then(function () {
                    vue_this.$call_remote_process("vehicle_order_center", "call_vehicle", [vue_this.$cookies.get("zh_ssid"), _vehicle.basic_info.id, _is_cancel]).then(function (resp) {
                        if (resp) {
                            vue_this.init_all_vehicle();
                        }
                    });
                });
            }
        },
        init_all_vehicle: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_registered_vehicle", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.all_vehicle = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_vehicle, index, element);
                    if (!vue_this.company_options.find(itr => {
                            return itr.value == element.basic_info.company_name
                        })) {
                        vue_this.company_options.push({
                            text: element.basic_info.company_name,
                            value: element.basic_info.company_name
                        });
                    }
                    if (!vue_this.stuff_options.find(itr => {
                            return itr.value == element.basic_info.stuff_name
                        })) {
                        vue_this.stuff_options.push({
                            text: element.basic_info.stuff_name,
                            value: element.basic_info.stuff_name
                        });
                    }
                });
            });
        },
    },
    beforeMount: function () {
        this.init_all_vehicle();
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}

.single_vehicle_show {
    border-bottom: 1px solid black;
}
</style>
