<template>
<view>
    <u-cell title="推送队列" url="/pages/req_que" is_link></u-cell>
    <u-upload accept="file" @afterRead="upload_update_pack">
        <u-cell title="上传更新包"></u-cell>
    </u-upload>
    <u-button type="error" text="退出登录" @click="log_off"></u-button>
    <u-divider></u-divider>
    <u-link :href="dev_page" text="打开维护页面"></u-link>
    <u-modal :show="confirm_update.length != 0" @confirm="exec_update" closeOnClickOverlay title="确定要升级吗?" @close="confirm_update = ''"></u-modal>
</view>
</template>

<script>
export default {
    data: function () {
        return {
            confirm_update: '',
        };
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
    methods: {
        exec_update: function () {
            console.log(this.confirm_update);
            this.$send_req('/api/update', {
                file: this.confirm_update
            }).finally(() => {
                this.confirm_update = '';
                uni.showToast({
                    title: '升级成功',
                    duration: 2000
                });
            });
        },
        upload_update_pack: function (_file) {
            console.log(_file);
            uni.showLoading({
                title: '正在上传...',
                mask: true
            });
            uni.uploadFile({
                url: 'https://' + uni.getStorageSync('domain') + '.d8sis.cn/upload/',
                files: [{
                    uri: _file.file.url,
                }],
                success: (res) => {
                    this.confirm_update = res.data.split('\r\n')[3].split('/').pop();
                },
                fail: (err) => {
                    console.log(err);
                    uni.showToast({
                        title: '上传失败',
                        duration: 2000
                    });
                },
                complete: () => {
                    uni.hideLoading();
                }
            });
        },
    },
}
</script>

<style>

</style>
