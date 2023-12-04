<template>
<view>
    <u-cell  title="推送队列" url="/pages/req_que" is_link ></u-cell>
    <u-button type="error" text="退出登录" @click="log_off"></u-button>
    <u-divider ></u-divider>
    <u-link :href="dev_page" text="打开维护页面"></u-link>
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
