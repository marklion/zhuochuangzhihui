<template>
<div class="all_came_vehicle_show">
    <van-nav-bar class="nav_bar_show" title="现场车辆管理" @click-left="show_opt_cell = true" left-text="现场" @click-right="init_all_vehicle" right-text="刷新" />
    <van-tabs v-model="act_page">
        <van-tab title="已排车辆">
            <van-dropdown-menu>
                <van-dropdown-item v-model="company_filter" :options="company_options" />
                <van-dropdown-item v-model="stuff_filter" :options="stuff_options" />
            </van-dropdown-menu>
            <van-search v-model="search_key" placeholder="车牌号过滤" />
            <div class="single_vehicle_show" v-for="(single_vehicle, index) in vehicle_need_show" :key="index">
                <van-cell :title="single_vehicle.basic_info.company_name" :label="single_vehicle.basic_info.stuff_name" :value="single_vehicle.basic_info.main_vehicle_number" center>
                </van-cell>
                <van-cell :title="single_vehicle.basic_info.driver_name" :value="single_vehicle.basic_info.driver_phone">
                    <template #label>
                        <div>
                            排号时间:{{single_vehicle.checkin_time}}
                        </div>
                        <div>
                            叫号时间:{{single_vehicle.call_time}}
                        </div>
                    </template>
                </van-cell>
                <van-cell title="附件" v-if="single_vehicle.basic_info.attachment">
                    <van-image width="100" height="100" :src="$remote_file_url + single_vehicle.basic_info.attachment" @click="preview_attachment(single_vehicle.basic_info.attachment)" />
                </van-cell>
                <van-cell title="叫号状态" :value="single_vehicle.has_called?'已叫号':'未叫号'" center>
                    <template #right-icon>
                        <div style="margin-left:10px;">
                            <van-button v-if="!single_vehicle.has_called" size="small" type="info" @click="call_vehicle(single_vehicle, false)">叫号</van-button>
                            <div v-else>
                                <van-button size="small" type="warning" @click="call_vehicle(single_vehicle, true)">取消叫号</van-button>
                                <van-button v-if="single_vehicle.basic_info.status == 1" size="small" type="danger" @click="pass_vehicle(single_vehicle)">过号</van-button>
                            </div>
                            <van-button v-if="single_vehicle.has_called && !single_vehicle.confirmed" size="small" type="primary" @click="confirm_deliver(single_vehicle)">确认装卸货</van-button>
                        </div>
                    </template>
                </van-cell>
                <van-cell v-if="single_vehicle.has_called" title="详细信息" is-link :to="{name:'FieldOpt', params:{order_no:single_vehicle.basic_info.order_number}}"></van-cell>
            </div>
        </van-tab>
        <van-tab title="自动叫号">
            <van-cell center v-for="(single_stuff, index) in all_stuff_info" :key="index" :title="single_stuff.name">
                <span>场内容量</span>
                <span>
                    <van-stepper min="0" :value="single_stuff.auto_call_count" async-change @change="change_auto_call_count" :name="single_stuff.name" />
                </span>
            </van-cell>
        </van-tab>
    </van-tabs>
    <van-popup v-model="show_opt_cell" position="bottom">
        <van-collapse v-model="device_opt_show" accordion>
            <van-collapse-item v-for="(single_cam, index) in all_cam_ips" :key="index" :title="single_cam.name" :name="index">
                <van-button type="info" size="small" @click="trigger_cap(single_cam.ip)">触发识别</van-button>
                <van-button type="primary" v-if="single_cam.is_scale" size="small" @click="manual_confirm_scale(single_cam.ip)">手动确认称重</van-button>
                <van-button type="warning" size="small" @click="trigger_gate_opt(single_cam.ip, 1)">开闸</van-button>
                <van-button type="danger" size="small" @click="trigger_gate_opt(single_cam.ip, 0)">关闸</van-button>
                <van-button type="info" size="small" @click="open_trigger_vehicle_diag(single_cam)">手动输入车号触发</van-button>
                <van-button type="primary" size="small" @click="get_cam_pic(single_cam.ip)">拍照</van-button>
            </van-collapse-item>
        </van-collapse>
    </van-popup>

    <van-dialog v-model="show_trigger_vehicle_number" title="输入车号触发" :showConfirmButton="false" closeOnClickOverlay>
        <van-field v-model="trigger_vehicle" center clearable label="车牌号" placeholder="请输入车牌">
            <template #button>
                <van-button size="small" type="primary" @click="trigger_vehicle_number(trigger_device_ip, trigger_device_name)">触发</van-button>
            </template>
        </van-field>
    </van-dialog>

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
            trigger_device_ip: '',
            trigger_device_name: '',
            trigger_vehicle: '',
            show_trigger_vehicle_number: false,
            all_stuff_info: [],
            act_page: 0,
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
                    ip: element.entry_config.cam_ip,
                    is_scale: false,
                });
                ret.push({
                    name: element.name + ' 出口',
                    ip: element.exit_config.cam_ip,
                    is_scale: false,
                });
            });
            this.device_config.scale.forEach((element) => {
                ret.push({
                    name: element.name + ' 入口',
                    ip: element.entry_config.cam_ip,
                    is_scale: true,
                });
                ret.push({
                    name: element.name + ' 出口',
                    ip: element.exit_config.cam_ip,
                    is_scale: true,
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
        get_cam_pic: function (_ip) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_cam_pic", [vue_this.$cookies.get("zh_ssid"), _ip]).then(function (resp) {
                ImagePreview([resp]);
            });
        },
        open_trigger_vehicle_diag: function (_device) {
            this.trigger_device_ip = _device.ip;
            this.trigger_device_name = _device.name.split(' ').slice(0, -1).join('');
            this.show_trigger_vehicle_number = true;
        },
        trigger_vehicle_number: function (_ip, _name) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", 'trigger_cam_vehicle_number', [vue_this.$cookies.get("zh_ssid"), vue_this.trigger_vehicle, _ip, _name]).then(function () {
                vue_this.show_trigger_vehicle_number = false;
                vue_this.trigger_vehicle = '';
            });
        },
        change_auto_call_count: function (_value, _detail) {
            console.log(_value);
            console.log(_detail);
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", "set_auto_call_count", [vue_this.$cookies.get('zh_ssid'), _detail.name, _value]).then(function (resp) {
                if (resp) {
                    vue_this.init_all_stuff();
                }
            });
        },
        init_all_stuff: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", "get_all_stuff", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_stuff_info, index, element);
                });
            });
        },
        pass_vehicle: function (_vehicle) {
            var vue_this = this;

            vue_this.$dialog.confirm({
                title: '过号确认',
                message: '确认过号吗？'
            }).then(function () {
                vue_this.$call_remote_process("vehicle_order_center", "driver_check_in", [parseInt(_vehicle.basic_info.id), true]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_vehicle();
                    }
                });

            });
        },
        confirm_deliver: function (_vehicle) {
            var vue_this = this;

            vue_this.$call_remote_process("vehicle_order_center", "confirm_order_deliver", [vue_this.$cookies.get("zh_ssid"), _vehicle.basic_info.order_number, true]).then(function (resp) {
                if (resp) {
                    vue_this.init_all_vehicle();
                }
            });
        },
        trigger_gate_opt: function (gate_code, _cmd) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", 'ctrl_gate', [gate_code, _cmd]).then(function (resp) {
                if (resp) {
                    vue_this.$message({
                        message: '操作成功',
                        type: 'success',
                    });
                } else {
                    vue_this.$message({
                        message: '操作失败',
                        type: 'error',
                    });
                }
            });
        },
        manual_confirm_scale: function (_scale_cam_ip) {
            var vue_this = this;
            var scale_name = "";
            vue_this.device_config.scale.forEach(element => {
                if (element.entry_config.cam_ip == _scale_cam_ip || _scale_cam_ip == element.exit_config.cam_ip) {
                    scale_name = element.name;
                }
            });
            vue_this.$call_remote_process("system_management", "manual_confirm_scale", [vue_this.$cookies.get("zh_ssid"), scale_name]).finally(function () {
                vue_this.$toast("已确认称重");
                vue_this.show_opt_cell = false;
            });
        },
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
        this.init_all_stuff();
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

.all_came_vehicle_show {
    overflow-y: scroll;
}
</style>
