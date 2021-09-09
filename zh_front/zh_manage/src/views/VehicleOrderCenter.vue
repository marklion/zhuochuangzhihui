<template>
<div class="vehicle_order_center_show">
    <el-row type="flex" justify="space-between" align="middle">
        <el-col>
            <div class="block_title_show">派车中心</div>
        </el-col>
        <el-col>
            <div align="right" style="margin-right:10px;">
                <el-button size="mini" type="success" icon="el-icon-plus" @click="current_opt_add=true;show_edit_order_diag = true">新增</el-button>
            </div>
        </el-col>
    </el-row>
    <el-tabs v-model="activeName">
        <el-tab-pane label="所有派车单" name="all">
            <el-table class="order_table_show" :data="all_order" style="width: 100%" stripe ref="order_table" v-infinite-scroll="get_order" :infinite-scroll-disabled="lazy_finish">
                <el-table-column prop="order_number" label="单号" min-width="22px">
                </el-table-column>
                <el-table-column prop="company_name" label="对方公司" min-width="60px">
                </el-table-column>
                <el-table-column prop="stuff_name" label="运输货物" min-width="40px">
                </el-table-column>
                <el-table-column prop="status" label="状态" min-width="20px" :formatter="status_show">
                </el-table-column>
                <el-table-column label="车辆信息" min-width="80px">
                    <template slot-scope="scope">
                        <el-descriptions size="mini" :column="2" border>
                            <el-descriptions-item label="主车">{{scope.row.main_vehicle_number}}</el-descriptions-item>
                            <el-descriptions-item label="挂车">{{scope.row.behind_vehicle_number}}</el-descriptions-item>
                            <el-descriptions-item label="司机">{{scope.row.driver_name}}</el-descriptions-item>
                            <el-descriptions-item label="电话">{{scope.row.driver_phone}}</el-descriptions-item>
                            <el-descriptions-item label="身份证">{{scope.row.driver_id}}</el-descriptions-item>
                        </el-descriptions>
                    </template>
                </el-table-column>
                <el-table-column fixed="right" label="操作" min-width="50px">
                    <template slot-scope="scope">
                        <el-button type="warning" size="mini">修改</el-button>
                        <el-button type="danger" size="mini" @click="console.log(scope)">删除</el-button>
                    </template>
                </el-table-column>
            </el-table>
        </el-tab-pane>
    </el-tabs>
    <el-dialog @close="clean_order" :title="(current_opt_add?'新增':'修改') + '派车单'" :visible.sync="show_edit_order_diag" width="60%" @keyup.enter.native="edit_order">
        <el-row type="flex" justify="space-between" align="middle">
            <el-col :span="10">
                <el-form :model="focus_order" ref="edit_order_form" :rules="rules" label-width="120px">
                    <el-form-item label="对方公司" prop="company_name">
                        <item-for-select v-model="focus_order.company_name" search_key="company_name"></item-for-select>
                    </el-form-item>
                    <el-form-item label="运输货物" prop="stuff_name">
                        <item-for-select v-model="focus_order.stuff_name" search_key="stuff_name"></item-for-select>
                    </el-form-item>
                    <el-form-item>
                        <el-button type="primary" @click="edit_order">确认</el-button>
                    </el-form-item>
                </el-form>
            </el-col>
            <el-col :span="12">
                <div class="all_vehicle_select_show">
                    <el-button type="success" size="mini" @click="show_vehicle_select = true">增加</el-button>
                    <el-empty v-if="vehicle_selected.length <= 0" description="未添加车辆"></el-empty>
                    <div v-else>
                        <div class="single_vehicle_show" v-for="(single_vehicle, index) in vehicle_selected" :key="index">
                            <el-descriptions size="mini" :column="2" border :title="single_vehicle.main_vehicle_number + '-' + single_vehicle.behind_vehicle_number">
                                <template slot="extra">
                                    <el-button type="danger" size="mini" @click="remove_single_vehicle(index)">移除</el-button>
                                </template>
                                <el-descriptions-item label="司机">{{single_vehicle.driver_name}}</el-descriptions-item>
                                <el-descriptions-item label="电话">{{single_vehicle.driver_phone}}</el-descriptions-item>
                                <el-descriptions-item label="身份证">{{single_vehicle.driver_id}}</el-descriptions-item>
                            </el-descriptions>
                        </div>
                    </div>
                </div>
            </el-col>
        </el-row>
    </el-dialog>
    <el-drawer @closed="clean_select" title="请选择车辆" :visible.sync="show_vehicle_select" direction="rtl" size="70%">
        <el-table :data="vehicle_for_select" style="width: 100%" ref="vehicle_select_table" stripe @selection-change="proc_select">
            <el-table-column type="selection" width="55" :selectable="verify_selectable">
            </el-table-column>
            <el-table-column sortable property="main_vehicle_number" label="主车牌" width="120"></el-table-column>
            <el-table-column sortable property="behind_vehicle_number" label="挂车牌" width="120"></el-table-column>
            <el-table-column sortable property="driver_name" label="司机" width="100"></el-table-column>
            <el-table-column sortable property="driver_phone" label="司机电话" width="120"></el-table-column>
            <el-table-column sortable property="driver_id" label="司机身份证" width="150"></el-table-column>
            <el-table-column sortable property="company_name" label="所属公司" width="150"></el-table-column>
            <el-table-column sortable property="group_name" label="分组名" width="100"></el-table-column>
        </el-table>
        <div align="right" style="margin-right:10px;">
            <el-button type="primary" @click="push_ready_to_select">确认</el-button>
        </div>
    </el-drawer>
</div>
</template>

<script>
import ItemForSelect from "../components/ItemForSelect.vue"
export default {
    name: 'VehicleOrderCenter',
    components: {
        "item-for-select": ItemForSelect,
    },
    data: function () {
        return {
            activeName: 'all',
            lazy_finish: false,
            ready_vehicle: [],
            show_vehicle_select: false,
            rules: {
                company_name: [{
                    required: true,
                    message: "请选择对方公司",
                    trigger: 'blur'
                }],
                stuff_name: [{
                    required: true,
                    message: "请选择运送的货物",
                    trigger: 'blur'
                }],
            },
            vehicle_selected: [],
            vehicle_for_select: [],
            current_opt_add: false,
            show_edit_order_diag: false,
            all_order: [],
            focus_order: {
                company_name: '',
                main_vehicle_number: '',
                behind_vehicle_number: '',
                driver_id: '',
                driver_phone: '',
                driver_name: '',
                stuff_name: '',
            },
            status_show: function (status) {
                var ret = "未进场";
                switch (status) {
                    case 1:
                        ret = '已进场';
                        break;
                    case 2:
                        ret = '已一次称重';
                        break;
                    case 3:
                        ret = '已二次称重';
                        break;
                    case 4:
                        ret = '已完成';
                        break;
                    default:
                        break;
                }
                return ret;
            },
        };
    },
    methods: {
        remove_single_vehicle: function (index) {
            this.vehicle_selected.splice(index, 1);
        },
        verify_selectable: function (row) {
            var ret = true;
            if (this.vehicle_selected.find((itr => {
                    return itr.id == row.id
                }))) {
                ret = false;
            }

            return ret;
        },
        clean_select: function () {
            this.ready_vehicle = [];
            this.$refs.vehicle_select_table.clearSelection();
        },
        push_ready_to_select: function () {
            var vue_this = this;
            this.ready_vehicle.forEach(element => {
                vue_this.vehicle_selected.push(element);
            });
            vue_this.show_vehicle_select = false;
        },
        proc_select: function (_value) {
            this.ready_vehicle = _value;
        },
        clean_order: function () {
            this.focus_order = {
                company_name: '',
                main_vehicle_number: '',
                behind_vehicle_number: '',
                driver_id: '',
                driver_phone: '',
                driver_name: '',
                stuff_name: '',
            }
            this.vehicle_selected = [];
        },
        refresh_order: function () {
            var vue_this = this;
            vue_this.lazy_finish = false;
            vue_this.all_order = [];
        },
        edit_order: function () {
            var vue_this = this;
            var req_body = {};
            var func_name = "";
            if (vue_this.current_opt_add) {
                req_body = [];
                vue_this.vehicle_selected.forEach(element => {
                    var single_req = element;
                    single_req.company_name = vue_this.focus_order.company_name;
                    single_req.stuff_name = vue_this.focus_order.stuff_name;
                    req_body.push(single_req);
                });
                func_name = "create_vehicle_order";
            } else {
                req_body = vue_this.focus_order;
            }
            vue_this.$refs.edit_order_form.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("vehicle_order_center", func_name, [vue_this.$cookies.get("zh_ssid"), req_body]).then(function (resp) {
                        if (resp) {
                            vue_this.refresh_order();
                            vue_this.show_edit_order_diag = false;
                        }
                    });

                }
            });
        },
        get_order: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_order_by_anchor", [vue_this.$cookies.get("zh_ssid"), vue_this.all_order.length]).then(function (resp) {
                resp.forEach(element => {
                    vue_this.all_order.push(element);
                });
                if (resp.length <= 0) {
                    vue_this.lazy_finish = true;
                }
            });
        },
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("vehicle_management", "get_all_vehicle", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
            resp.forEach((element, index) => {
                vue_this.$set(vue_this.vehicle_for_select, index, element);
            });
        });
    },
}
</script>

<style scoped>
.single_vehicle_show {
    border: 1px solid black;
    margin-top: 10px;
}

.all_vehicle_select_show {
    max-height: 400px;
    overflow: auto;
}

.order_table_show {
    height: 75vh;
    overflow: auto;
}
</style>
