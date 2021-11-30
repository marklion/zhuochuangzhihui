<template>
<div class="vehicle_order_center_show">
    <el-row type="flex" justify="space-between" align="middle">
        <el-col>
            <div class="block_title_show">派车中心</div>
        </el-col>
        <el-col>
            <div align="right" style="margin-right:10px;">
                <el-button size="mini" type="primary" icon="el-icon-notebook-2" v-if="order_selected.length > 0" @click="export_xlsx">导出所选{{order_selected.length}}项</el-button>
                <el-button size="mini" type="success" icon="el-icon-plus" @click="current_opt_add=true;show_edit_order_diag = true">新增</el-button>
            </div>
        </el-col>
    </el-row>
    <el-row type="flex" justify="space-between">
        <el-col>
            <el-tabs v-model="activeName">
                <el-tab-pane label="所有派车单" name="all">
                </el-tab-pane>
                <el-tab-pane label="未确认" name="need_confirm">
                </el-tab-pane>
                <el-tab-pane label="未入场" name="has_not_come">
                </el-tab-pane>
                <el-tab-pane label="已入场" name="insite">
                </el-tab-pane>
                <el-tab-pane label="已完成" name="end">
                </el-tab-pane>
            </el-tabs>
        </el-col>
        <el-col :span="8">
            <div align="right" style="margin-right:10px;">
                <el-input v-model="search_condition" placeholder="输入公司名拼音首字母/车牌号过滤" prefix-icon="el-icon-search"></el-input>
            </div>
        </el-col>
    </el-row>
    <el-table :data="order_need_show" @selection-change="proc_order_select" style="width: 100%" stripe ref="order_table" infinite-scroll-listen-for-event="need_refresh" v-infinite-scroll="get_order" :infinite-scroll-disabled="need_fetch" :infinite-scroll-distance="10">
        <el-table-column type="selection" width="30px">
        </el-table-column>
        <el-table-column prop="order_number" label="单号" min-width="22px">
        </el-table-column>
        <el-table-column prop="company_name" label="公司" min-width="25px">
        </el-table-column>
        <el-table-column prop="stuff_name" label="运输货物" min-width="20px">
        </el-table-column>
        <el-table-column label="状态" min-width="120px">
            <template slot-scope="scope">
                <el-steps :active="scope.row.status">
                    <el-step v-for="(single_status, index) in scope.row.status_details" :key="index">
                        <template slot="title">
                            <div class="step_title">
                                {{single_status.name}}
                            </div>
                        </template>
                        <template slot="description">
                            <div class="step_description_name">
                                {{single_status.user_name}}
                            </div>
                            <div class="step_description_time">
                                {{single_status.timestamp.split(" ")[0]}}
                            </div>
                            <div class="step_description_time">
                                {{single_status.timestamp.split(" ")[1]}}
                            </div>
                        </template>
                    </el-step>
                </el-steps>
            </template>
        </el-table-column>
        <el-table-column label="车辆信息" min-width="80px">
            <template slot-scope="scope">
                <el-descriptions size="mini" :column="2" border>
                    <el-descriptions-item label="主车">{{scope.row.main_vehicle_number}}</el-descriptions-item>
                    <el-descriptions-item label="挂车">{{scope.row.behind_vehicle_number}}</el-descriptions-item>
                    <el-descriptions-item label="一次称重">{{scope.row.p_weight}}</el-descriptions-item>
                    <el-descriptions-item label="二次称重">{{scope.row.m_weight}}</el-descriptions-item>
                    <el-descriptions-item label="司机">{{scope.row.driver_name}}</el-descriptions-item>
                    <el-descriptions-item label="电话">{{scope.row.driver_phone}}</el-descriptions-item>
                    <el-descriptions-item label="身份证">{{scope.row.driver_id}}</el-descriptions-item>
                </el-descriptions>
            </template>
        </el-table-column>
        <el-table-column fixed="right" label="操作" min-width="20px">
            <template slot-scope="scope">
                <div>
                    <el-button type="warning" size="mini">修改</el-button>
                </div>
                <div v-if="scope.row.status < 2">
                    <el-button type="danger" size="mini" @click="cancel_order([scope.row])">取消</el-button>
                </div>
                <div v-if="$store.state.user_info.permission <= 1 && scope.row.status == 0">
                    <el-button type="success" size="mini" @click="confirm_order([scope.row])">确认可进</el-button>
                </div>
            </template>
        </el-table-column>
    </el-table>
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
import Vue from 'vue';
import ItemForSelect from "../components/ItemForSelect.vue"
import infiniteScroll from 'vue-infinite-scroll'

import PinyinMatch from "pinyin-match"
import XLSX from 'xlsx';
Vue.use(infiniteScroll)
export default {
    name: 'VehicleOrderCenter',
    components: {
        "item-for-select": ItemForSelect,
    },
    events: {
        ['need_refresh']() {
            console.log("tick");
        }
    },
    computed: {
        need_fetch:function() {
            var ret = false;
            if (!this.lazy_finish && !this.is_loading)
            {
                ret = true;
            }
            return ret;
        },
        order_need_show: function () {
            var ret = [];

            this.all_order.forEach(element => {
                switch (this.activeName) {
                    case 'all':
                        ret.push(element);
                        break;
                    case 'need_confirm':
                        if (element.status == 0) {
                            ret.push(element);
                        }
                        break;
                    case 'has_not_come':
                        if (element.status == 1) {
                            ret.push(element);
                        }
                        break;

                    case 'insite':
                        if (element.status > 1 && element.status < 100) {
                            ret.push(element);
                        }
                        break;
                    case 'end':
                        if (element.status == 100) {
                            ret.push(element);
                        }
                        break;
                    default:
                        break;
                }
            });
            var tmp_ret = ret;
            ret = [];
            tmp_ret.forEach(element => {
                if (this.search_condition.length <= 0) {
                    ret.push(element);
                } else if (PinyinMatch.match(element.company_name, this.search_condition) || PinyinMatch.match(element.main_vehicle_number, this.search_condition)) {
                    ret.push(element);
                }
            });

            return ret;
        },
    },
    data: function () {
        return {
            order_selected: [],
            search_condition: '',
            activeName: 'all',
            lazy_finish: false,
            is_loading: false,
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

        };
    },
    methods: {
        proc_order_select: function (_value) {
            this.order_selected = _value;
        },
        cancel_order: function (orders) {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "cancel_vehicle_order", [vue_this.$cookies.get("zh_ssid"), orders]).then(function (resp) {
                if (resp) {
                    vue_this.refresh_order();
                }
            });
        },
        confirm_order: function (orders) {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "confirm_vehicle_order", [vue_this.$cookies.get("zh_ssid"), orders]).then(function (resp) {
                if (resp) {
                    vue_this.refresh_order();
                }
            });
        },
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
            vue_this.all_order = [];
            vue_this.lazy_finish = false;
            this.$nextTick(() => {
                vue_this.$emit("need_refresh");
            });
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
            vue_this.is_loading = true;
            vue_this.$call_remote_process("vehicle_order_center", "get_order_by_anchor", [vue_this.$cookies.get("zh_ssid"), vue_this.all_order.length]).then(function (resp) {
                resp.forEach(element => {
                    vue_this.all_order.push(element);
                });
                if (resp.length <= 0) {
                    vue_this.lazy_finish = true;
                }
            }).finally(function() {
                vue_this.is_loading = false;
            });
        },
        // 导出xlsx
        exportExcel: function (headers, data, fileName = '记录表.xlsx') {
            const _headers = headers
                .map((item, i) => Object.assign({}, {
                    key: item.key,
                    title: item.title,
                    position: String.fromCharCode(65 + i) + 1
                }))
                .reduce((prev, next) => Object.assign({}, prev, {
                    [next.position]: {
                        key: next.key,
                        v: next.title
                    }
                }), {});

            const _data = data
                .map((item, i) => headers.map((key, j) => Object.assign({}, {
                    content: item[key.key],
                    position: String.fromCharCode(65 + j) + (i + 2)
                })))
                // 对刚才的结果进行降维处理（二维数组变成一维数组）
                .reduce((prev, next) => prev.concat(next))
                // 转换成 worksheet 需要的结构
                .reduce((prev, next) => Object.assign({}, prev, {
                    [next.position]: {
                        v: next.content
                    }
                }), {});

            // 合并 headers 和 data
            const output = Object.assign({}, _headers, _data);
            // 获取所有单元格的位置
            const outputPos = Object.keys(output);
            // 计算出范围 ,["A1",..., "H2"]
            const ref = `${outputPos[0]}:${outputPos[outputPos.length - 1]}`;

            // 构建 workbook 对象
            const wb = {
                SheetNames: ['mySheet'],
                Sheets: {
                    mySheet: Object.assign({},
                        output, {
                            '!ref': ref,
                            '!cols': [{
                                wpx: 80
                            }, {
                                wpx: 180
                            }, {
                                wpx: 70
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 60
                            }, {
                                wpx: 120
                            }, {
                                wpx: 60
                            }, {
                                wpx: 120
                            }, {
                                wpx: 80
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }],
                        },
                    ),
                },
            };

            // 导出 Excel
            XLSX.writeFile(wb, fileName);
        },
        export_xlsx: function () {
            var init_colm = [{
                title: '编号',
                key: 'order_number',
            }, {
                title: '公司',
                key: 'company_name',
            }, {
                title: '车号',
                key: 'main_vehicle_number',
            }, {
                title: "创建时间",
                key: 'create_time',
            }, {
                title: "确认时间",
                key: 'confirm_time',
            }, {
                title: "进场时间",
                key: 'enter_time',
            }, {
                title: "出场时间",
                key: 'exit_time',
            }, {
                title: "关闭时间",
                key: 'close_time',
            }, {
                title: "一次称重",
                key: 'p_weight',
            }, {
                title: "一次称重时间",
                key: 'p_time',
            }, {
                title: "二次称重",
                key: 'm_weight',
            }, {
                title: "二次称重时间",
                key: 'm_time',
            }, {
                title: "司机",
                key: 'driver_name',
            }, {
                title: "电话",
                key: 'driver_phone',
            }, {
                title: "身份证",
                key: 'driver_id',
            }, ];
            var content = [];
            this.order_selected.forEach(element => {
                var tmp = {
                    ...element
                };
                tmp.status_details.forEach(single_status => {
                    switch (single_status.step) {
                        case 0:
                            tmp.create_time = single_status.timestamp;
                            break;
                        case 1:
                            tmp.confirm_time = single_status.timestamp;
                            break;
                        case 2:
                            tmp.enter_time = single_status.timestamp;
                            break;
                        case 3:
                            tmp.p_time = single_status.timestamp;
                            break;
                        case 4:
                            tmp.m_time = single_status.timestamp;
                            break;
                        case 5:
                            tmp.exit_time = single_status.timestamp;
                            break;
                        case 100:
                            tmp.close_time = single_status.timestamp;
                            break;
                        default:
                            break;
                    }
                });
                content.push(tmp);
            });
            this.exportExcel(init_colm, content);
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

.vehicle_order_center_show {
    height: 100%;
    overflow: auto;
}

.step_title {
    font-size: 10px;
}

.step_description_name {
    font-size: 8px;
    color: green;
}

.step_description_time {
    font-size: 6px;
    color: red;
}
</style>
