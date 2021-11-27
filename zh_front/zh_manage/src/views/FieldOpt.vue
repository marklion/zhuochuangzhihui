<template>
<div class="field_opt_show">
    <van-nav-bar class="nav_bar_show" title="装卸货确认" />
    <van-cell title="京AB1234" label="京AB123挂" value="动力煤"></van-cell>
    <van-cell title="张三" value="13600001111" label="尔林兔煤矿"></van-cell>
    <div style="margin:15px;">
        <van-button block type="primary">确认</van-button>
    </div>
</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import 'vant/lib/index.css';
Vue.use(Vant);

export default {
    name: 'FieldOpt',
    data: function () {
        return {};
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}
</style>
