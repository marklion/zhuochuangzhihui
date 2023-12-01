<template>
<view>
    <u-link :href="dev_page" text="打开维护页面"></u-link>
    <u-button type="error" text="退出登录" @click="log_off"></u-button>
</view>
</template>

<script>
export default {
    data: function () {
        return {};
    },
    computed: {
        dev_page: function () {
            var ret = '';

            ret = "https://" + uni.getStorageSync('domain') + ".d8sis.cn/wetty";

            return ret;
        },
        log_off: function () {
            uni.removeStorageSync('token');
            uni.reLaunch({
                url: '/pages/login'
            })
        },
    },
}
</script>

<style>

</style>
