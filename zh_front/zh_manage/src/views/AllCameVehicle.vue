<template>
<div class="all_came_vehicle_show">
    <van-nav-bar class="nav_bar_show" title="现场车辆管理" @click-left="show_opt_cell = true" left-text="现场" @click-right="init_all_vehicle" right-text="刷新" />
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
    <van-popup v-model="show_opt_cell" position="bottom">
        <van-collapse v-model="device_opt_show" accordion>
            <van-collapse-item v-for="(single_cam, index) in all_cam_ips" :key="index" :title="single_cam.name" :name="index">
                <van-button type="info" size="small" @click="trigger_cap(single_cam.ip)">触发识别</van-button>
            </van-collapse-item>
        </van-collapse>
    </van-popup>

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
            show_opt_cell: false,
            device_opt_show: '',
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
            device_config: {
                gate: [],
                scale: [],
            },
        };
    },
    computed: {
        all_cam_ips: function () {
            var ret = [];

            this.device_config.gate.forEach((element) => {
                ret.push({
                    name: element.name + ' 入口',
                    ip: element.entry_config.cam_ip
                });
                ret.push({
                    name: element.name + ' 出口',
                    ip: element.exit_config.cam_ip
                });
            });
            this.device_config.scale.forEach((element) => {
                ret.push({
                    name: element.name + ' 入口',
                    ip: element.entry_config.cam_ip
                });
                ret.push({
                    name: element.name + ' 出口',
                    ip: element.exit_config.cam_ip
                });
            });

            return ret;
        },
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
        trigger_cap: function (_cam_ip) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "trigger_cap", [vue_this.$cookies.get("zh_ssid"), _cam_ip]).then(function () {
                vue_this.show_opt_cell = false;
            });
        },
        get_device_config: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_device_config", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.device_config = resp;
            });
        },
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
        this.get_device_config();
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
