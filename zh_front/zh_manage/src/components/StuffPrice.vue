<template>
<div class="stuff_price_show">
    <el-skeleton v-if="is_loading" :rows="2" animated />
    <el-descriptions :column="1" size="mini" v-else :title="stuff_name">
        <el-descriptions-item label="单价">{{price.toFixed(2)}}</el-descriptions-item>
        <el-descriptions-item label="预计装车净重">{{expect_weight.toFixed(2)}}</el-descriptions-item>
    </el-descriptions>

</div>
</template>

<script>
export default {
    name: 'StuffPrice',
    data: function () {
        return {
            is_loading: true,
            price: 0,
            expect_weight: 0,
        };
    },
    props: {
        stuff_name: String,
    },
    watch:{
        stuff_name:function () {
            this.init_price_data();
        },
    },
    methods: {
        init_price_data: function () {
            var vue_this = this;
            vue_this.is_loading = true;
            vue_this.price = 0;
            vue_this.expect_weight = 0;
            vue_this.$call_remote_process_no_toast("stuff_management", "get_stuff", [vue_this.$cookies.get("zh_ssid"), vue_this.stuff_name]).then(function (resp) {
                vue_this.price = resp.price;
                vue_this.expect_weight = resp.expect_weight;
            }).finally(function () {
                vue_this.is_loading = false;
            });
        },
    },
    beforeMount: function () {
        this.init_price_data();
    },
}
</script>

<style>

</style>
