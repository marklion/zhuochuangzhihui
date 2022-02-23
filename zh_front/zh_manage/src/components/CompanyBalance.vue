<template>
<div class="company_balance_show">
    <el-skeleton v-if="is_loading" :rows="2" animated />
    <el-descriptions :column="1" size="mini" v-else :title="company_name">
        <el-descriptions-item label="余额">{{balance.toFixed(2)}}</el-descriptions-item>
        <el-descriptions-item label="授信">{{credit.toFixed(2)}}</el-descriptions-item>
    </el-descriptions>
</div>
</template>

<script>
export default {
    name: 'CompanyBalance',
    data: function () {
        return {
            is_loading: true,
            balance: 0,
            credit: 0,
        };
    },
    props: {
        company_name: String,
    },
    watch: {
        company_name: function () {
            this.init_data();
        },
    },
    methods: {
        init_data: function () {
            var vue_this = this;
            vue_this.balance = 0;
            vue_this.credit = 0;
            vue_this.is_loading = true;
            vue_this.$call_remote_process_no_toast("contract_management", "get_contract", [vue_this.$cookies.get("zh_ssid"), vue_this.company_name]).then(function (resp) {
                vue_this.balance = resp.balance;
                vue_this.credit = resp.credit;
            }).finally(function () {
                vue_this.is_loading = false;
            });
        },
    },
    beforeMount: function () {
        this.init_data();
    },
}
</script>

<style>

</style>
