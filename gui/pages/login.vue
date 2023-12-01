<template>
<view>
    <u--form labelPosition="left" :model="login_req" ref="uForm">
        <u-form-item label="电话" prop="phone" borderBottom>
            <u--input v-model="login_req.phone" border="none"></u--input>
        </u-form-item>
        <u-form-item label="密码" prop="password" borderBottom>
            <u--input v-model="login_req.password" type="password" border="none"></u--input>
        </u-form-item>
    </u--form>
    <u-button @click="login" type="primary">提交</u-button>
</view>
</template>

<script>
export default {
    data: function () {
        return {
            login_req: {
                username: '',
                password: '',
            },
        };
    },
    methods: {
        login: function () {
            var vue_this = this;
            const crypto = require('crypto');
            var md5_pwd = crypto.createHash('md5').update(vue_this.login_req.password).digest('hex').toUpperCase();
            vue_this.$send_req('/api/login', {
                phone: vue_this.login_req.phone,
                pwd: md5_pwd
            }).then(resp => {
                uni.setStorageSync('token', resp);
                uni.reLaunch({
                    url: '/'
                })
            });
        },
    },
}
</script>

<style>

</style>
