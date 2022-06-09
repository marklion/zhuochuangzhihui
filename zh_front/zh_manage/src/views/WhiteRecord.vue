<template>
<div class="white_record_show">
    <el-date-picker v-model="date_range" type="daterange" range-separator="至" start-placeholder="开始日期" end-placeholder="结束日期" @change="fetch_recored">
    </el-date-picker>
    <el-table :data="record" style="width: 100%" stripe ref="order_table">
        <el-table-column type="index" label="编号" min-width="10px">
        </el-table-column>
        <el-table-column label="车号" width="250px" prop="vehicle_number">
        </el-table-column>
        <el-table-column label="日期" width="250px" prop="date">
        </el-table-column>
        <el-table-column label="重量" width="250px" prop="weight">
        </el-table-column>
    </el-table>
</div>
</template>

<script>
export default {
    name: 'WhiteRecord',
    data: function () {
        return {
            begin_date: new Date(new Date().setDate(new Date().getDate() - 7)),
            end_date: new Date(),
            record: [],
        };
    },
    computed: {
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
