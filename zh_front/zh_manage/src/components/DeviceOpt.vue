<template>
<div class="device_opt_show">
    <van-cell :title="device_status.name">
        <template #label v-if="device_status.is_scale">
            <div>
                <van-tag v-if="!device_status.enter_gate_is_close" plain type="danger">入口未关</van-tag>
                <van-tag v-else plain type="success">入口已关</van-tag>
                <van-tag v-if="!device_status.exit_gate_is_close" plain type="danger">出口未关</van-tag>
                <van-tag v-else plain type="success">出口已关</van-tag>
            </div>
            <div>
                {{device_status.cur_status}}
            </div>
        </template>
        <div v-if="device_status.is_scale">
            {{device_status.cur_weight}}吨
        </div>
        <template #right-icon v-if="device_status.is_scale">
            <van-button size="mini" type="primary" @click="confirm_weight">确认称重</van-button>
            <van-button size="mini" type="danger" @click="reset_scale">重置</van-button>
        </template>
    </van-cell>
    <van-row :gutter="5">
        <van-col span="6">
            <van-button size="small" block type="primary" @click="gate_control(true, false)">入口开闸</van-button>
        </van-col>
        <van-col span="6">
            <van-button size="small" block type="danger" @click="gate_control(true, true)">入口关闸</van-button>
        </van-col>
        <van-col span="6">
            <van-button size="small" block type="primary" @click="gate_control(false, false)">出口开闸</van-button>
        </van-col>
        <van-col span="6">
            <van-button size="small" block type="danger" @click="gate_control(false, true)">出口关闸</van-button>
        </van-col>
    </van-row>
    <van-row :gutter="5" type="flex" justify="space-between">
        <van-col span="6">
            <van-button size="small" block type="info" @click="take_picture(true)">入口拍照</van-button>
        </van-col>
        <van-col span="6">
            <van-button size="small" block type="warning" @click="trigger_diag_show = true;focus_is_enter=true">入口触发</van-button>
        </van-col>
        <van-col span="6">
            <van-button size="small" block type="info" @click="take_picture(false)">出口拍照</van-button>
        </van-col>
        <van-col span="6">
            <van-button size="small" block type="warning" @click="trigger_diag_show = true;focus_is_enter=false">出口触发</van-button>
        </van-col>
    </van-row>
    <van-dialog v-model="trigger_diag_show" title="触发识别" closeOnClickOverlay @confirm="manual_trigger(focus_is_enter, trigger_vehicle)">
        <van-field v-model="trigger_vehicle" label="车牌" placeholder="不输入车牌则拍照触发" />
    </van-dialog>
</div>
</template>

<script>
import {
    ImagePreview
} from 'vant';
export default {
    name: 'DeviceOpt',
    props: {
        device_status: Object
    },
    data: function () {
        return {
            focus_is_enter: false,
            trigger_diag_show: false,
            trigger_vehicle: '',
        };
    },
    methods: {
        pub_event() {
            this.$emit("refresh");
        },
        gate_control: function (_is_enter, _is_close) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "gate_control", [vue_this.device_status.name, _is_enter, _is_close, vue_this.$cookies.get("zh_ssid")]).then(function () {
                vue_this.pub_event();
            });
        },
        take_picture: function (_is_enter) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "take_picture", [vue_this.device_status.name, _is_enter, vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                ImagePreview([resp]);
                vue_this.pub_event();
            });
        },
        manual_trigger: function (_is_enter, _vehicle_number) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "manual_trigger", [vue_this.device_status.name, _is_enter, _vehicle_number, vue_this.$cookies.get("zh_ssid")]).then(function () {
                vue_this.pub_event();
            });
        },
        confirm_weight: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "confirm_weight", [vue_this.device_status.name, vue_this.$cookies.get("zh_ssid")]).then(function () {
                vue_this.pub_event();
            });
        },
        reset_scale: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "reset_sm", [vue_this.device_status.name, vue_this.$cookies.get("zh_ssid")]).then(function () {
                vue_this.pub_event();
            });
        },
    },
}
</script>

<style>

</style>
