<template>
<div class="check_in_show">
    <van-cell :title="cur_vehicle.basic_info.main_vehicle_number" :label="cur_vehicle.basic_info.company_name" :value="cur_vehicle.basic_info.stuff_name"></van-cell>
    <van-cell :title="cur_vehicle.basic_info.driver_name" :value="cur_vehicle.basic_info.driver_phone" :label="cur_vehicle.basic_info.driver_id"></van-cell>
    <van-field readonly clickable name="picker" :value="cur_vehicle.basic_info.source_dest_name" :label="source_or_dest" placeholder="点击选择" @click="show_source_dest_picker = true" />
    <van-popup v-model="show_source_dest_picker" position="bottom">
        <van-picker show-toolbar :columns="all_source_dest" @confirm="select_source_dest" @cancel="show_source_dest_picker = false" />
    </van-popup>
    <van-cell title="当前状态" :value="status" center>
        <template #right-icon>
            <van-button size="small" type="info" icon="replay" @click="init_vehicle">刷新</van-button>
        </template>
        <template #label>
            <div v-if="cur_vehicle.registered">
                {{cur_vehicle.checkin_time}}
            </div>
        </template>
    </van-cell>
    <vue-qr :text="cur_url" :size="200"></vue-qr>
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
    <van-dialog v-model="show_direction" confirm-button-text="知道了" title="进厂指引" closeOnClickOverlay>
        <van-swipe :autoplay="3000">
            <van-swipe-item v-for="(image, index) in all_prompt_img" :key="index">
                <img v-lazy="$remote_file_url + image.attachment_path" width="150" height="300" />
            </van-swipe-item>
        </van-swipe>
    </van-dialog>
    <div style="margin:15px;">
        <van-button v-if="!cur_vehicle.registered" block type="info" @click="driver_check_in(false)">排号</van-button>
        <van-button v-else block type="warning" @click="driver_check_in(true)">取消排号</van-button>
        <van-divider />
        <van-button block type="danger" @click="driver_cancel_order">取消派车</van-button>
    </div>
</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import {
    Lazyload
} from 'vant';

Vue.use(Lazyload);

// 注册时可以配置额外的选项
Vue.use(Lazyload, {
    lazyComponent: true,
});
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
            show_direction: false,
            domain_name: '',
            show_source_dest_picker: false,
            all_prompt_img: [],
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

            show_upload_diag: false,
            all_source_dest: [],
        };
    },
    computed: {
        cur_url: function () {
            var order_number = '';
            if (this.cur_vehicle.basic_info.order_number) {
                order_number = this.cur_vehicle.basic_info.order_number;
            }
            return 'http://' + this.domain_name + '/#/mobile/field_opt/' + order_number;
        },
        source_or_dest: function () {
            var ret = '物料源';
            if (this.cur_vehicle.basic_info.is_sale) {
                ret = '物料目的地';
            }

            return ret;
        },
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
        driver_cancel_order: function () {
            var vue_this = this;
            vue_this.$dialog.confirm({
                title: "取消确认",
                message: "确定要取消该派车单吗？"
            }).then(function () {
                vue_this.$call_remote_process("vehicle_order_center", "cancel_vehicle_order", [vue_this.cur_vehicle.basic_info.driver_phone, [{
                    id: vue_this.cur_vehicle.basic_info.id
                }]]).then(function (resp) {
                    if (resp) {
                        vue_this.$router.go(0);
                    }
                });
            });
        },
        select_source_dest: function (_value) {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", 'record_order_source_dest', [vue_this.cur_vehicle.basic_info.id, _value]).then(function (resp) {
                if (resp) {
                    vue_this.init_vehicle();
                    vue_this.show_source_dest_picker = false;
                }
            });
        },
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
        getDistance(lat1, lng1, lat2, lng2) {
            console.log(lat1);
            console.log(lng1);
            console.log(lat2);
            console.log(lng2);
            var dis = 0;
            var radLat1 = toRadians(lat1);
            var radLat2 = toRadians(lat2);
            var deltaLat = radLat1 - radLat2;
            var deltaLng = toRadians(lng1) - toRadians(lng2);
            dis = 2 * Math.asin(Math.sqrt(Math.pow(Math.sin(deltaLat / 2), 2) + Math.cos(radLat1) * Math.cos(radLat2) * Math.pow(Math.sin(deltaLng / 2), 2)));
            return dis * 6378137;

            function toRadians(d) {
                return d * Math.PI / 180;
            }
        },
        getLocation(options = {}) {
            const resultPromise = new Promise((resolve, reject) => {
                if (navigator.geolocation && navigator.geolocation.getCurrentPosition) {
                    navigator.geolocation.getCurrentPosition((res) => {
                        resolve(res)
                    }, (err) => {
                        reject(err)
                    }, options)
                } else {
                    reject({
                        code: 1,
                        message: "Browser doesn't support"
                    })
                }
            })
            return resultPromise
        },
        check_postion: async function () {
            var vue_this = this;
            return new Promise(async function (resolve, reject) {
                try {
                    var self_pos = await vue_this.getLocation();
                    var config_pos = await vue_this.$call_remote_process("system_management", "get_company_address_info", []);
                    if (config_pos.distance == 0) {
                        resolve();
                    } else {
                        var calc_distance = vue_this.getDistance(self_pos.coords.latitude, self_pos.coords.longitude, config_pos.y, config_pos.x);
                        if (config_pos.distance >= calc_distance) {
                            resolve();
                        } else {
                            reject("未进入指定区域，距离(" + calc_distance.toFixed(2) + "米)过远")
                        }
                    }
                } catch (error) {
                    console.log(error);
                    reject("获取定位失败")
                }
            });
        },
        driver_check_in: function (_cancel) {
            var vue_this = this;
            if (!_cancel) {
                if (!vue_this.cur_vehicle.basic_info.is_sale) {
                    vue_this.cur_vehicle.basic_info.source_dest_name = vue_this.cur_vehicle.basic_info.company_name;
                }
                vue_this.check_postion().then(function () {
                    vue_this.$call_remote_process("vehicle_order_center", "driver_check_in", [parseInt(vue_this.cur_vehicle.basic_info.id), _cancel]).then(function (resp) {
                        if (resp) {
                            vue_this.init_vehicle();
                            vue_this.show_direction = true;
                        }
                    }).catch(function (error) {
                        vue_this.$dialog.alert({
                            title: '无法排号',
                            message: error.msg
                        })
                    });
                }).catch(function (msg) {
                    vue_this.$toast(msg);
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
                vue_this.$call_remote_process("stuff_management", 'get_all_source_dest', [!resp.basic_info.is_sale]).then(function (sd_resp) {
                    vue_this.all_source_dest = [];
                    sd_resp.forEach((element, index) => {
                        vue_this.$set(vue_this.all_source_dest, index, element.name);
                    });
                })
            });
        },
        init_prompt_img: function () {
            var vue_this = this;

            vue_this.$call_remote_process("system_management", "get_all_prompt_image", []).then(function (resp) {
                vue_this.all_prompt_img = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_prompt_img, index, element);
                });
            });
        },
    },
    beforeMount: function () {
        this.init_vehicle();
        this.init_prompt_img();
        var vue_this = this;
        vue_this.$call_remote_process("system_management", "get_domain_name", []).then(function (resp) {
            vue_this.domain_name = resp;
        });
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}
</style>
