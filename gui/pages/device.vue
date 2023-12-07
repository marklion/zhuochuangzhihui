<template>
<view>
    <u-divider text="称重状态"></u-divider>
    <view v-for="(single_sm, index) in scale_sm" :key="index">
        <u-cell-group :title="single_sm.set_info.name + '-' + single_sm.cur_state + '-' + single_sm.cur_plate">
            <u-cell>
                <view slot="label">
                    <u-row gutter="10" justify="space-between">
                        <u-col :span="3">
                            <u-button type="primary" size="mini" text="开前门" @click="gate_ctrl(single_sm.set_info.gate.front.id, true)"></u-button>
                        </u-col>
                        <u-col :span="3">
                            <u-button type="primary" size="mini" text="开后门" @click="gate_ctrl(single_sm.set_info.gate.back.id, true)"></u-button>
                        </u-col>
                        <u-col :span="3">
                            <u-button type="error" size="mini" text="关前门" @click="gate_ctrl(single_sm.set_info.gate.front.id, false)"></u-button>
                        </u-col>
                        <u-col :span="3">
                            <u-button type="error" size="mini" text="关后门" @click="gate_ctrl(single_sm.set_info.gate.back.id, false)"></u-button>
                        </u-col>
                    </u-row>
                </view>
                <view slot="title">
                    <u-row gutter="10" justify="space-between">
                        <u-col :span="4">
                            重量：{{single_sm.cur_weight.toFixed(2)}}
                        </u-col>
                        <u-col :span="4">
                            <u-tag text="前门已关" v-if="single_sm.front_gate_is_close" plain shape="circle" size="mini" type="success"></u-tag>
                            <u-tag text="前门未关" v-else plain shape="circle" size="mini" type="error"></u-tag>
                        </u-col>
                        <u-col :span="4">
                            <u-tag text="后门已关" v-if="single_sm.back_gate_is_close" plain shape="circle" size="mini" type="success"></u-tag>
                            <u-tag text="后门未关" v-else plain shape="circle" size="mini" type="error"></u-tag>
                        </u-col>
                    </u-row>
                </view>
            </u-cell>
            <u-cell>
                <view slot="title">
                    <u-grid :col="3">
                        <u-grid-item>
                            <u-button type="success" size="mini" text="入口拍照" @click="take_pic(single_sm.set_info.plate_cam.front.id)"></u-button>
                        </u-grid-item>
                        <u-grid-item>
                            <u-button type="primary" size="mini" text="入口识别" @click="focus_cam_id = single_sm.set_info.plate_cam.front.id"></u-button>
                        </u-grid-item>
                        <u-grid-item>
                            <u-button type="warning" size="mini" text="手动称重" @click="manual_scale(single_sm.set_info.id)"></u-button>
                        </u-grid-item>
                        <u-grid-item>
                            <u-button type="success" size="mini" text="出口拍照" @click="take_pic(single_sm.set_info.plate_cam.back.id)"></u-button>
                        </u-grid-item>
                        <u-grid-item>
                            <u-button type="primary" size="mini" text="出口识别" @click="focus_cam_id = single_sm.set_info.plate_cam.back.id"></u-button>
                        </u-grid-item>
                        <u-grid-item>
                            <u-button type="error" size="mini" text="重置" @click="reset_confirm_id =  single_sm.set_info.id"></u-button>
                        </u-grid-item>
                    </u-grid>
                </view>
            </u-cell>
        </u-cell-group>
    </view>
    <u-popup :show="pic_path.length != 0" mode="center" @close="pic_path = ''">
        <view>
            <u--image :src="pic_path"></u--image>
        </view>
    </u-popup>
    <u-modal :show="focus_cam_id != 0" title="请输入车牌号" closeOnClickOverlay @close="focus_cam_id = 0" @confirm="trigger_cap">
        <view>
            <u--input placeholder="不输入即直接抓拍识别" border="surround" v-model="focus_plate"></u--input>
        </view>
    </u-modal>
    <u-modal :show="reset_confirm_id != 0" title="确定要重置吗?" closeOnClickOverlay @close="reset_confirm_id = 0" @confirm="reset_scale">
    </u-modal>
    <u-divider text="设备状态"></u-divider>
    <u-read-more toggle closeText="展开更多" v-if="all_device.length > 0" :showHeight="200">
        <u-cell v-for="(single_dev, index) in all_device" :key="index" :title="single_dev.name" :value="single_dev.stay_time">
        </u-cell>
    </u-read-more>
</view>
</template>

<script>
export default {
    data: function () {
        return {
            scale_sm: [],
            pic_path: '',
            focus_cam_id: 0,
            focus_plate: '',
            reset_confirm_id: 0,
            all_device: [],
        }
    },

    onPullDownRefresh() {
        this.init_device_info();
    },
    onLoad() {
        this.init_device_info();
    },
    methods: {
        reset_scale: function () {
            this.$send_req('/api/reset_scale_sm', {
                sm_id: this.reset_confirm_id,
            }).then(() => {
                this.reset_confirm_id = 0;
                uni.startPullDownRefresh();
            });
        },
        manual_scale: function (_sm_id) {
            this.$send_req('/api/confirm_scale', {
                sm_id: _sm_id,
            }).then(() => {
                uni.startPullDownRefresh();
            });
        },
        trigger_cap: function () {
            var cb_func = () => {
                this.focus_cam_id = 0;
                this.focus_plate = '';
                uni.startPullDownRefresh();
            };
            if (this.focus_plate) {
                this.$send_req('/api/device_mock/push_plate', {
                    device_id: this.focus_cam_id,
                    plate_no: this.focus_plate,
                }).then(cb_func);
            } else {
                this.$send_req('/api/plate_cam_cap', {
                    device_id: this.focus_cam_id,
                }).then(cb_func);
            }
        },
        take_pic: function (device_id) {
            this.$send_req("/api/cap_picture", {
                device_id: device_id,
            }).then(resp => {
                this.pic_path = 'https://' + uni.getStorageSync('domain') + '.d8sis.cn' + resp;
            });
        },
        gate_ctrl: function (device_id, _is_open) {
            this.$send_req('/api/gate_ctrl', {
                device_id: device_id,
                is_open: _is_open,
            }).then(() => {
                uni.startPullDownRefresh();
            });
        },
        init_stay_time: function () {
            this.$send_req('/api/device_run_time', {}).then(resp => {
                this.all_device = resp;
            });
        },
        init_device_info: async function () {
            await this.init_stay_time();
            this.$send_req('/api/get_scale_sm_info', {}).then(resp => {
                this.scale_sm = resp;
                uni.stopPullDownRefresh();
            });
        },
    },
}
</script>

<style>

</style>
