<template>
<div class="check_in_show">
    <van-cell :title="cur_vehicle.basic_info.main_vehicle_number" :label="cur_vehicle.basic_info.company_name" :value="cur_vehicle.basic_info.stuff_name"></van-cell>
    <van-cell :title="cur_vehicle.basic_info.driver_name" :value="cur_vehicle.basic_info.driver_phone" :label="cur_vehicle.basic_info.driver_id"></van-cell>
    <van-cell title="当前状态" :value="status" center>
        <template #right-icon>
            <van-button size="small" type="info" icon="replay" @click="init_vehicle">刷新</van-button>
        </template>
    </van-cell>
    <vue-qr :text="cur_url()" :size="200"></vue-qr>
    <div v-if="cur_vehicle.basic_info.need_enter_weight">
        <van-cell title="矿(厂)发净重" clickable is-link @click="show_upload_diag = true" :value="cur_vehicle.basic_info.enter_weight" center>
        </van-cell>
        <van-cell title="矿(厂)发磅单" clickable is-link @click="show_attachment([$remote_file_url + cur_vehicle.basic_info.enter_weight_attachment]);" center>
        </van-cell>
        <van-dialog @close="clean_uplaod" v-model="show_upload_diag" title="上传磅单" :showConfirmButton="false" closeOnClickOverlay>
            <el-form ref="upload_detail" :model="upload_detail" label-width="100px">
                <el-form-item label="矿(厂)发净重" prop="enter_weight" :rules="[{ required: true, message: '请填写净重'}, {validator:weight_should_not_0}]">
                    <el-input v-model="upload_detail.enter_weight" type="number"></el-input>
                </el-form-item>
                <el-form-item label="磅单" prop="enter_weight_attachment" :rules="[{ required: true, message: '请上传磅单'}]">
                    <el-upload list-type="picture" accept="image/*" :action="$remote_url + '/upload/'" :limit="1" :on-success="make_attach_name">
                        <el-button type="success" size="small">上传</el-button>
                    </el-upload>
                </el-form-item>
                <el-form-item>
                    <el-button type="primary" @click="upload_enter_weight">提交</el-button>
                </el-form-item>
            </el-form>
        </van-dialog>
    </div>
    <div style="margin:15px;">
        <van-button v-if="!cur_vehicle.registered" block type="info" @click="driver_check_in(false)">排号</van-button>
        <van-button v-else block type="danger" @click="driver_check_in(true)">取消排号</van-button>
    </div>
</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import 'vant/lib/index.css';
import vueQr from 'vue-qr'
Vue.use(Vant);
import {
    ImagePreview
} from 'vant';
export default {
    name: 'CheckIn',
    components: {
        vueQr
    },
    props: {
        driver_phone: String,
    },
    watch: {
        driver_phone: function (_new_phone) {
            console.log(_new_phone);
            this.init_vehicle();
        }
    },
    data: function () {
        return {
            upload_detail: {
                enter_weight: 0,
                enter_weight_attachment: '',
            },
            cur_vehicle: {
                basic_info: {},
                confirmed: false,
                registered: false,
                has_called: false,
            },
            cur_url: function () {
                return document.location.href;
            },
            show_upload_diag: false,

        };
    },
    computed: {
        status: function () {
            var ret = "未排号";
            if (this.cur_vehicle.registered) {
                ret = '已排号, 等待叫号';
            }
            if (this.cur_vehicle.has_called) {
                ret = '已叫号, 可以进厂';
            }

            return ret;
        },
    },
    methods: {
        weight_should_not_0: (rule, value, callback) => {
            if (parseFloat(value) > 0) {
                callback();
            } else {
                callback(new Error('请输入有效净重'));
            }
            console.log(rule);
        },
        show_attachment: function (_attachment) {
            ImagePreview(_attachment);
        },
        clean_uplaod: function () {
            this.upload_detail = {
                enter_weight: 0,
                enter_weight_attachment: '',
            };
        },
        upload_enter_weight: function () {
            var vue_this = this;
            vue_this.$refs.upload_detail.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("vehicle_order_center", "upload_enter_weight_attachment", [vue_this.cur_vehicle.basic_info.id, vue_this.upload_detail.enter_weight_attachment, vue_this.upload_detail.enter_weight]).then(function (resp) {
                        if (resp) {
                            vue_this.init_vehicle();
                            vue_this.show_upload_diag = false;
                        }
                    });
                }
            });
        },
        make_attach_name: function (resp, file) {
            var real_path = resp.match(/^\/tmp\/.*/gm)[0];
            var file_name_split = file.name.split('.');
            this.upload_detail.enter_weight_attachment = real_path + '.' + file_name_split[file_name_split.length - 1];
        },
        driver_check_in: function (_cancel) {
            var vue_this = this;
            if (!_cancel) {
                vue_this.$call_remote_process("vehicle_order_center", "driver_check_in", [parseInt(vue_this.cur_vehicle.basic_info.id), _cancel]).then(function (resp) {
                    if (resp) {
                        vue_this.init_vehicle();
                    }
                });
            } else {
                vue_this.$dialog.confirm({
                    title: "取消确认",
                    message: "确认取消排号吗？"
                }).then(function () {
                    vue_this.$call_remote_process("vehicle_order_center", "driver_check_in", [parseInt(vue_this.cur_vehicle.basic_info.id), _cancel]).then(function (resp) {
                        if (resp) {
                            vue_this.init_vehicle();
                        }
                    });
                });
            }

        },
        init_vehicle() {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "driver_get_order", [vue_this.driver_phone]).then(function (resp) {
                vue_this.cur_vehicle = resp;
                vue_this.$set(vue_this.cur_vehicle, 'basic_info', resp.basic_info);
            });
        },
    },
    beforeMount: function () {
        this.init_vehicle();
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}
</style>
