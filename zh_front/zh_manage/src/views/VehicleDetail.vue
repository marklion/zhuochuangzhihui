<template>
<div class="vehicle_detail_show">
    <el-image-viewer v-if="show_enter_weight" :on-close="()=>{show_enter_weight=false}" :url-list="[$remote_file_url + cur_vehicle.basic_info.enter_weight_attachment]" />
    <el-row type="flex" align="middle" :gutter="40">
        <el-col :span="12">
            <el-descriptions title="基本信息" :column="2" border>
                <el-descriptions-item label="派车单号">{{cur_vehicle.basic_info.order_number}}</el-descriptions-item>
                <el-descriptions-item label="派车公司">{{cur_vehicle.basic_info.company_name}}</el-descriptions-item>
                <el-descriptions-item label="拉运货物">{{cur_vehicle.basic_info.stuff_name}}</el-descriptions-item>
                <el-descriptions-item label="主车">{{cur_vehicle.basic_info.main_vehicle_number}}</el-descriptions-item>
                <el-descriptions-item label="挂车">{{cur_vehicle.basic_info.behind_vehicle_number}}</el-descriptions-item>
                <el-descriptions-item v-if="cur_vehicle.basic_info.need_enter_weight" label="矿(厂)发净重">
                    <span>
                        {{cur_vehicle.basic_info.enter_weight}}
                    </span>
                    <span>
                        <el-button type="text" @click="show_enter_weight = true">预览磅单</el-button>
                    </span>
                </el-descriptions-item>
            </el-descriptions>
        </el-col>
        <el-col :span="12">
            <el-descriptions title="司机信息" :column="3" border>
                <el-descriptions-item label="司机">{{cur_vehicle.basic_info.driver_name}}</el-descriptions-item>
                <el-descriptions-item label="司机电话">{{cur_vehicle.basic_info.driver_phone}}</el-descriptions-item>
                <el-descriptions-item label="司机身份证">{{cur_vehicle.basic_info.driver_id}}</el-descriptions-item>
            </el-descriptions>
            <el-descriptions title="称重数据" :column="2" border>
                <el-descriptions-item label="一次称重">
                    {{cur_vehicle.basic_info.p_weight}}
                </el-descriptions-item>
                <el-descriptions-item label="二次称重">
                    {{cur_vehicle.basic_info.m_weight}}
                </el-descriptions-item>
                <template slot="extra">
                    <div style="padding:5px;" v-if="$store.state.user_info.permission != 3">
                        <el-button size="mini" type="danger" @click="manual_close">手动结束运输</el-button>
                        <el-button type="primary" size="mini" @click="manual_weight_show = true">手动修改称重</el-button>
                    </div>
                </template>
            </el-descriptions>
        </el-col>
    </el-row>
    <el-dialog title="手动修改" :visible.sync="manual_weight_show" width="40%" @open="assign_weight">
        <el-row :gutter="10" type="flex" align="middle">
            <el-col :span="6">
                一次称重
            </el-col>
            <el-col :span="10">
                <el-input v-model="manual_weight_data.p_weight"></el-input>
            </el-col>
            <el-col :span="6">
                <el-button size="mini" type="primary" @click="manual_p_weight_edit">修改</el-button>
            </el-col>
        </el-row>
        <el-row :gutter="10" type="flex" align="middle">
            <el-col :span="6">
                二次称重
            </el-col>
            <el-col :span="10">
                <el-input v-model="manual_weight_data.m_weight"></el-input>
            </el-col>
            <el-col :span="6">
                <el-button size="mini" type="primary" @click="manual_m_weight_edit">修改</el-button>
            </el-col>
        </el-row>
    </el-dialog>

    <el-row type="flex" :gutter="5">
        <el-col :span="6">
            <div class="status_show">
                <el-divider>流程</el-divider>
                <div style="flex-grow:1;">
                    <el-steps :active="cur_vehicle.basic_info.status" direction="vertical">
                        <el-step v-for="(single_status, index) in cur_vehicle.basic_info.status_details" :key="index" :title="single_status.name" :description="single_status.user_name + ' ' + single_status.timestamp">
                        </el-step>
                    </el-steps>
                </div>
            </div>
        </el-col>
        <el-col :span="12">
            <div class="status_show">
                <el-divider>状态</el-divider>
                <el-row type="flex">
                    <el-col>
                        <el-result :icon="cur_vehicle.registered?'success':'info'" :title="cur_vehicle.registered?'已排号':'未排号'">
                            <template slot="extra">
                                <div v-if="!cur_vehicle.registered">
                                    <el-button :disabled="cur_vehicle.basic_info.status == 100" type="success" size="medium" @click="driver_check_in(false)">代替司机排号</el-button>
                                </div>
                                <div v-else>
                                    <el-button :disabled="cur_vehicle.basic_info.status == 100" type="danger" size="medium" @click="driver_check_in(true)">取消排号</el-button>
                                </div>
                            </template>
                        </el-result>
                    </el-col>
                    <el-col v-if="cur_vehicle.registered">
                        <el-result :icon="cur_vehicle.has_called?'success':'info'" :title="cur_vehicle.has_called?'已叫号':'未叫号'">
                            <template slot="extra">
                                <div v-if="!cur_vehicle.has_called">
                                    <el-button :disabled="cur_vehicle.basic_info.status == 100 || $store.state.user_info.permission == 3" type="success" size="medium" @click="call_vehicle(false)">叫号</el-button>
                                </div>
                                <div v-else>
                                    <el-button :disabled="cur_vehicle.basic_info.status == 100 || $store.state.user_info.permission == 3" type="danger" size="medium" @click="call_vehicle(true)">取消叫号</el-button>
                                </div>
                            </template>
                        </el-result>
                    </el-col>
                    <el-col v-if="cur_vehicle.has_called">
                        <el-result :icon="cur_vehicle.confirmed?'success':'info'" :title="cur_vehicle.confirmed?'已确认装卸货':'未确认装卸货'">
                            <template slot="extra">
                                <div v-if="!cur_vehicle.confirmed">
                                    <el-button :disabled="cur_vehicle.basic_info.status == 100 || $store.state.user_info.permission == 3" type="success" size="medium" @click="confirm_deliver(true)">确认</el-button>
                                </div>
                                <div v-else>
                                    <el-button :disabled="cur_vehicle.basic_info.status == 100 || $store.state.user_info.permission == 3" type="danger" size="medium" @click="confirm_deliver(false)">取消确认</el-button>
                                </div>
                            </template>
                        </el-result>
                    </el-col>
                </el-row>
            </div>
        </el-col>
        <el-col :span="6">
            <div class="status_show">
                <el-divider>附件</el-divider>
                <div v-if="!cur_vehicle.basic_info.attachment">
                    <el-empty description="无附件"></el-empty>
                    <el-upload v-if="cur_vehicle.basic_info.status <=1" accept="image/*" :action="$remote_url + '/upload/'" :show-file-list="false" :limit="1" :on-success="upload_attachment">
                        <el-button size="small" type="primary">点击上传</el-button>
                    </el-upload>
                </div>
                <div v-else>
                    <el-image fit="scale-down" style="width: 100%; flex-grow:1; height:250px;" :src="$remote_file_url + cur_vehicle.basic_info.attachment" :preview-src-list="[$remote_file_url + cur_vehicle.basic_info.attachment]">
                    </el-image>
                    <el-button v-if="cur_vehicle.basic_info.status <=1" type="danger" size="medium" @click="delete_attachment">删除附件</el-button>
                </div>
            </div>
        </el-col>
    </el-row>

</div>
</template>

<script>
export default {
    name: "VehicleDetail",
    data: function () {
        return {
            show_enter_weight: false,
            cur_vehicle: {
                basic_info: {},
                confirmed: false,
                registered: false,
                has_called: false,
            },
            manual_weight_show: false,
            manual_weight_data: {
                p_weight: 0.0,
                m_weight: 0.0,
            },
        };
    },
    components: {
        'el-image-viewer': () => import('element-ui/packages/image/src/image-viewer')
    },
    methods: {
        assign_weight: function () {
            this.manual_weight_data.p_weight = this.cur_vehicle.basic_info.p_weight;
            this.manual_weight_data.m_weight = this.cur_vehicle.basic_info.m_weight;
        },
        manual_close() {
            var vue_this = this;
            vue_this.$confirm("确定要结束派车单吗？结束后会结算物料库存", "结束确认", {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("vehicle_order_center", "manual_close", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_vehicle.basic_info.id]).then(function (resp) {
                    if (resp) {
                        vue_this.init_cur_vehicle();
                        vue_this.manual_weight_show = false;
                    }
                });
            });
        },
        manual_p_weight_edit: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "manual_set_p_weight", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_vehicle.basic_info.id, vue_this.manual_weight_data.p_weight]).then(function (resp) {
                if (resp) {
                    vue_this.init_cur_vehicle();
                    vue_this.manual_weight_show = false;
                }
            });
        },
        manual_m_weight_edit: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "manual_set_m_weight", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_vehicle.basic_info.id, vue_this.manual_weight_data.m_weight]).then(function (resp) {
                if (resp) {
                    vue_this.init_cur_vehicle();
                    vue_this.manual_weight_show = false;
                }
            });
        },
        ask_user_first(_msg, _is_cancel) {
            var vue_this = this;
            if (!_is_cancel) {
                return new Promise(function (resolve) {
                    resolve(_is_cancel);
                });
            } else {
                return new Promise(function (resolve) {
                    vue_this.$confirm(_msg, "取消确认", {
                        confirmButtonText: '确定',
                        cancelButtonText: '取消',
                        type: 'warning'
                    }).then(() => {
                        resolve(_is_cancel);
                    });
                });
            }
        },
        confirm_deliver: function (_confirm) {
            var vue_this = this;
            vue_this.ask_user_first("确定取消吗?", !_confirm).then(function () {
                vue_this.$call_remote_process("vehicle_order_center", "confirm_order_deliver", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_vehicle.basic_info.order_number, _confirm]).then(function (resp) {
                    if (resp) {
                        vue_this.init_cur_vehicle();
                    }
                });
            });
        },
        call_vehicle: function (_is_cancel) {
            var vue_this = this;
            vue_this.ask_user_first("确定要取消叫号吗？", _is_cancel).then(function () {
                vue_this.$call_remote_process("vehicle_order_center", "call_vehicle", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_vehicle.basic_info.id, _is_cancel]).then(function (resp) {
                    if (resp) {
                        vue_this.init_cur_vehicle();
                    }
                });
            });
        },
        driver_check_in: function (_cancel) {
            var vue_this = this;
            vue_this.ask_user_first("确定要取消排号吗？若已叫号则会自动取消叫号", _cancel).then(function () {
                vue_this.$call_remote_process("vehicle_order_center", "driver_check_in", [parseInt(vue_this.cur_vehicle.basic_info.id), _cancel]).then(function (resp) {
                    if (resp) {
                        vue_this.init_cur_vehicle();
                    }
                });
            });
        },
        init_cur_vehicle: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_order_detail", [vue_this.$cookies.get("zh_ssid"), vue_this.$route.params.order_no]).then(function (resp) {
                vue_this.cur_vehicle = resp;
                vue_this.$set(vue_this.cur_vehicle, "basic_info", resp.basic_info);
            });
        },
        upload_attachment: function (resp, file) {
            var real_path = resp.match(/^\/tmp\/.*/gm)[0];
            var file_name_split = file.name.split('.');
            this.cur_vehicle.basic_info.attachment = real_path + '.' + file_name_split[file_name_split.length - 1];
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "update_vehicle_order", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_vehicle.basic_info]).finally(() => {
                vue_this.init_cur_vehicle();
            });
        },
        delete_attachment: function () {
            var vue_this = this;
            vue_this.cur_vehicle.basic_info.attachment = "";
            vue_this.$call_remote_process("vehicle_order_center", "update_vehicle_order", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_vehicle.basic_info]).finally(function () {
                vue_this.init_cur_vehicle();
            });
        },
    },
    beforeMount: function () {
        this.init_cur_vehicle();
    },
}
</script>

<style scoped>
.status_show {
    border: 1px solid black;
    margin: 10px;
    padding: 10px;
    height: 400px;
    display: flex;
    flex-direction: column;
}
</style>
