<template>
<div class="home">
    <el-row type="flex" :gutter="10">
        <el-col :span="10">
            <div class="dashboard_show">
                <el-divider content-position="left">派车单统计</el-divider>
                <ve-ring :data="{columns:vehicle_os_columns, rows:vehicle_os_row}" :title="{left:'center',text:'正在进行的派车单:共' + total_order + '单'}" :legend="{left:'right', orient:'vertical', top:'middle', formatter:match_number_name}" />
            </div>
        </el-col>
        <el-col :span="14">
            <div class="dashboard_show">
                <el-row type="flex" align="middle" :gutter="10">
                    <el-col>
                        <el-divider content-position="left">物料统计</el-divider>
                    </el-col>
                    <el-col>
                        <el-select v-model="focus_stuff" size="small" placeholder="请选择要统计的物料">
                            <el-option v-for="item in stuff_history" :key="item.stuff.name" :label="item.stuff.name" :value="item.stuff.name">
                            </el-option>
                        </el-select>
                    </el-col>
                    <el-col>
                        <el-date-picker size="small" @change="init_stuff_history" v-model="date_range" type="daterange" align="right" unlink-panels range-separator="至" start-placeholder="开始日期" end-placeholder="结束日期" :picker-options="pickerOptions">
                        </el-date-picker>
                    </el-col>
                </el-row>
                <ve-line :legend="{show:false}" :data="stuff_chart_data(focus_stuff)" :title="{left:'center', text:focus_stuff}"></ve-line>
            </div>
        </el-col>
    </el-row>
    <el-divider content-position="left">设备状态</el-divider>

    <vue-grid align="stretch" justify="start">
        <vue-cell v-for="(single_device, index) in all_device" :key="index" width="3of12">
            <device-opt :device_status="single_device" @refresh="init_all_device"></device-opt>
        </vue-cell>
    </vue-grid>
</div>
</template>

<script>
import {
    VueGrid,
    VueCell
} from 'vue-grd';
import DeviceOpt from "../components/DeviceOpt.vue"
export default {
    name: 'Home',
    data: function () {
        return {
            date_range: [
                new Date(new Date().getTime() - (7 * 24 * 3600 * 1000)),
                new Date()
            ],
            total_order: 0,
            vehicle_os_row: [],
            all_device: [],
            vehicle_os_columns: ['状态', '数量'],
            stuff_history: [],
            focus_stuff: '',
            stuff_chart_data: function (_name) {
                var ret = {
                    columns: ['时间', '库存']
                };
                this.stuff_history.forEach(element => {
                    if (element.stuff.name == _name) {
                        ret.rows = [];
                        element.change_point.forEach(one_point => {
                            ret.rows.push({
                                "时间": one_point.date,
                                "库存": one_point.inventory
                            });
                        });
                    }
                });
                return ret;
            },
            pickerOptions: {
                shortcuts: [{
                    text: '最近一周',
                    onClick(picker) {
                        const end = new Date();
                        const start = new Date();
                        start.setTime(start.getTime() - 3600 * 1000 * 24 * 7);
                        picker.$emit('pick', [start, end]);
                    }
                }, {
                    text: '最近一个月',
                    onClick(picker) {
                        const end = new Date();
                        const start = new Date();
                        start.setTime(start.getTime() - 3600 * 1000 * 24 * 30);
                        picker.$emit('pick', [start, end]);
                    }
                }, {
                    text: '最近三个月',
                    onClick(picker) {
                        const end = new Date();
                        const start = new Date();
                        start.setTime(start.getTime() - 3600 * 1000 * 24 * 90);
                        picker.$emit('pick', [start, end]);
                    }
                }]
            },
        };
    },
    components: {
        VueGrid,
        VueCell,
        'device-opt': DeviceOpt,
    },
    computed: {},
    methods: {
        match_number_name: function (name) {
            var number_of_name = this.vehicle_os_row.find((element) => {
                return element['状态'] == name;
            })['数量'];
            return name + ' ' + number_of_name + ' 单';
        },
        init_order_statistics: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_order_statistics", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.vehicle_os_row = [];
                Object.keys(resp).forEach((key) => {
                    if (key == "total") {
                        vue_this.total_order = resp[key];
                        return;
                    }
                    var single_rec = {};
                    switch (key) {
                        case 'created':
                            single_rec['状态'] = '待确认';
                            break;
                        case 'confirmed':
                            single_rec['状态'] = '待入场';
                            break;
                        case 'entered':
                            single_rec['状态'] = '待一次称重';
                            break;
                        case 'first_weight':
                            single_rec['状态'] = '待二次称重';
                            break;
                        case 'second_weight':
                            single_rec['状态'] = '待出厂';
                            break;
                        case 'total':
                            single_rec['状态'] = '全部';
                            break;
                        default:
                            break;
                    }
                    single_rec['数量'] = resp[key];
                    vue_this.vehicle_os_row.push(single_rec);
                });
            });
        },
        init_stuff_history: function () {
            var vue_this = this;
            var begin_date = vue_this.date_range[0];
            var end_date = new Date(vue_this.date_range[1]);
            end_date.setDate(end_date.getDate() + 1);
            vue_this.$call_remote_process("stuff_management", "get_change_points_for_range", [vue_this.$cookies.get("zh_ssid"), vue_this.$make_time_string(begin_date, '/'), vue_this.$make_time_string(end_date, '/')]).then(function (resp) {
                vue_this.stuff_history = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.stuff_history, index, element);
                });
            });
        },
        init_all_device: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_all_device", []).then(function (resp) {
                vue_this.all_device = [];
                resp.forEach((item, index) => {
                    vue_this.$set(vue_this.all_device, index, item);
                });
            });
        },
    },
    beforeMount: function () {
        this.init_order_statistics();
        this.init_stuff_history();
        var vue_this = this;
        vue_this.$call_remote_process("stuff_management", "get_last_active", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
            vue_this.focus_stuff = resp;
        });
        this.init_all_device();
    },
}
</script>

<style scoped>
.dashboard_show {
    padding: 2px;
    margin: 3px;
}

.device_health_name_show {
    font-size: 10px;
}

.scale_status_show {
    border: 1px solid;
}
</style>
