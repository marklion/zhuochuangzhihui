<template>
<div class="white_record_show">
    <el-row :gutter="10">
        <el-col :span="6">
            <el-date-picker v-model="date_range" type="daterange" range-separator="至" start-placeholder="开始日期" end-placeholder="结束日期" @change="fetch_recored">
            </el-date-picker>
        </el-col>
        <el-col :span="4">
            <el-input v-model="search_vehicle" placeholder="过滤车号"></el-input>
        </el-col>
        <el-col :span="4">
            <el-input v-model="search_group" placeholder="过滤分组"></el-input>
        </el-col>
        <el-col :span="6">
            {{record_need_show.length}}车, 共计{{vehicle_total_count}}吨.
        </el-col>
        <el-col :span="4">
            <table-import-export export_name="白名单称重导出表.xlsx" :export_table="record_need_show" :item_name_map="col_map()" :no_need_import="true"></table-import-export>
        </el-col>
    </el-row>
    <el-table :data="record_need_show" style="width: 100%" stripe ref="order_table">
        <el-table-column type="index" label="编号" min-width="10px">
        </el-table-column>
        <el-table-column label="车号" width="250px" prop="vehicle_number">
        </el-table-column>
        <el-table-column label="日期" width="250px" prop="date">
        </el-table-column>
        <el-table-column label="分组名" width="250px" prop="group_name">
        </el-table-column>
        <el-table-column label="重量" width="250px" prop="weight">
        </el-table-column>
    </el-table>
</div>
</template>

<script>
import PinyinMatch from "pinyin-match"
import TableImportExport from '../components/TableImportExport.vue'
export default {
    name: 'WhiteRecord',
    data: function () {
        return {
            search_vehicle: '',
            search_group: '',
            begin_date: new Date(new Date().setDate(new Date().getDate() - 7)),
            end_date: new Date(),
            record: [],
            col_map: function () {
                var ret = {
                    vehicle_number: {
                        text: '主车车号'
                    },
                    group_name: {
                        text: '分组名'
                    },
                    weight: {
                        text: '重量'
                    },
                    date: {
                        text: '日期'
                    },
                }
                return ret;
            },
        };
    },
    components: {
        "table-import-export": TableImportExport,
    },
    computed: {
        vehicle_total_count: function () {
            var ret = 0;

            this.record_need_show.forEach(element => {
                ret += parseFloat(element.weight)
            });

            return ret.toFixed(2);
        },
        record_need_show: function () {
            var ret = this.record;
            if (this.search_vehicle) {
                var tmp_ret = [];
                ret.forEach(element => {
                    if (PinyinMatch.match(element.vehicle_number, this.search_vehicle)) {
                        tmp_ret.push(element);
                    }
                });
                ret = tmp_ret;
            }
            if (this.search_group) {
                tmp_ret = [];
                ret.forEach(element => {
                    if (PinyinMatch.match(element.group_name, this.search_group)) {
                        tmp_ret.push(element);
                    }
                });
                ret = tmp_ret;
            }

            return ret;
        },
        date_range: {
            get: function () {
                var ret = [];
                ret.push(this.begin_date);
                ret.push(this.end_date);
                return ret;
            },
            set: function (_new_value) {
                this.begin_date = _new_value[0];
                this.end_date = _new_value[1];
            }
        },

    },
    methods: {
        fetch_recored: function () {
            var vue_this = this;
            console.log(vue_this.begin_date);
            vue_this.$call_remote_process("vehicle_order_center", "get_white_record_info", [vue_this.$cookies.get("zh_ssid"), parseInt(vue_this.begin_date.getTime() / 1000), parseInt(vue_this.end_date.getTime() / 1000)]).then(function (resp) {
                vue_this.record = [];
                resp.forEach(function (element, index) {
                    vue_this.$set(vue_this.record, index, element);
                });
            });
        },
    },
    beforeMount: function () {
        this.fetch_recored();
    }
}
</script>

<style>

</style>
