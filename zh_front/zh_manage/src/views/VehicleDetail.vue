<template>
<div class="vehicle_detail_show">
    <el-row type="flex" align="middle" :gutter="40">
        <el-col :span="12">
            <el-descriptions title="基本信息" :column="2" border>
                <el-descriptions-item label="派车单号">{{cur_vehicle.basic_info.order_number}}</el-descriptions-item>
                <el-descriptions-item label="派车公司">{{cur_vehicle.basic_info.company_name}}</el-descriptions-item>
                <el-descriptions-item label="拉运货物">{{cur_vehicle.basic_info.stuff_name}}</el-descriptions-item>
                <el-descriptions-item label="主车">{{cur_vehicle.basic_info.main_vehicle_number}}</el-descriptions-item>
                <el-descriptions-item label="挂车">{{cur_vehicle.basic_info.behind_vehicle_number}}</el-descriptions-item>
            </el-descriptions>
        </el-col>
        <el-col :span="12">
            <el-descriptions title="司机信息" :column="3" border>
                <el-descriptions-item label="司机">{{cur_vehicle.basic_info.driver_name}}</el-descriptions-item>
                <el-descriptions-item label="司机电话">{{cur_vehicle.basic_info.driver_phone}}</el-descriptions-item>
                <el-descriptions-item label="司机身份证">{{cur_vehicle.basic_info.driver_id}}</el-descriptions-item>
            </el-descriptions>
            <el-descriptions title="称重数据" :column="2" border>
                <el-descriptions-item label="一次称重">{{cur_vehicle.basic_info.p_weight}}</el-descriptions-item>
                <el-descriptions-item label="二次称重">{{cur_vehicle.basic_info.m_weight}}</el-descriptions-item>
            </el-descriptions>
        </el-col>
    </el-row>

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
                    <el-col :span="8">
                        <el-result :icon="cur_vehicle.registered?'success':'info'" :title="cur_vehicle.registered?'已排号':'未排号'">
                            <template slot="extra">
                                <div v-if="!cur_vehicle.registered">
                                    <el-button type="success" size="medium">代替司机排号</el-button>
                                </div>
                                <div v-else>
                                    <el-button type="danger" size="medium">取消排号</el-button>
                                </div>
                            </template>
                        </el-result>
                    </el-col>
                    <el-col :span="8">
                        <el-result :icon="cur_vehicle.has_called?'success':'info'" :title="cur_vehicle.has_called?'已叫号':'未叫号'">
                            <template slot="extra">
                                <div v-if="!cur_vehicle.has_called">
                                    <el-button type="success" size="medium">叫号</el-button>
                                </div>
                                <div v-else>
                                    <el-button type="danger" size="medium">取消叫号</el-button>
                                </div>
                            </template>
                        </el-result>
                    </el-col>
                    <el-col :span="8">
                        <el-result :icon="cur_vehicle.confirmed?'success':'info'" :title="cur_vehicle.confirmed?'已确认装卸货':'未确认装卸货'">
                            <template slot="extra">
                                <div v-if="!cur_vehicle.confirmed">
                                    <el-button type="success" size="medium">确认</el-button>
                                </div>
                                <div v-else>
                                    <el-button type="danger" size="medium">取消确认</el-button>
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
                    <el-image style="width: 100%; flex-grow:1;" :src="$remote_file_url + cur_vehicle.basic_info.attachment" :preview-src-list="[$remote_file_url + cur_vehicle.basic_info.attachment]">
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
            cur_vehicle: {
                basic_info: {},
                confirmed: false,
                registered: false,
                has_called: false,
            },
        };
    },
    methods: {
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
            vue_this.$call_remote_process("vehicle_order_center", "update_vehicle_order", [vue_this.$cookies.get("zh_ssid"), vue_this.cur_vehicle.basic_info]).finally(()=>{
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
