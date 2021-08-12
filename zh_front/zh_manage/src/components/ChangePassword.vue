<template>
<div class="change_password_show">
    <el-form :model="user_input" ref="user_input" :rules="rules" label-width="100px" @keyup.enter.native="change_password">
        <el-form-item label="新密码" prop="new_password">
            <el-input v-model="user_input.new_password" show-password placeholder="请输入密码"></el-input>
        </el-form-item>
        <el-form-item label="确认密码" prop="new_password_confirm">
            <el-input v-model="user_input.new_password_confirm" show-password placeholder="请再次输入密码"></el-input>
        </el-form-item>
        <el-form-item>
            <el-button type="primary" @click="change_password">提交</el-button>
        </el-form-item>
    </el-form>
</div>
</template>

<script>
export default {
    name: 'ChangePassword',
    data: function () {
        return {
            user_input: {
                new_password: '',
                new_password_confirm: '',
            },
            rules: {
                new_password: [{
                        required: true,
                        message: '请输入新密码',
                        trigger: 'blur'
                    },
                    {
                        pattern: /^(?![^a-zA-Z]+$)(?!\D+$).{8,15}$/,
                        message: '字母+数字组合,至少包含一个字母和数字,长度控制在8-15',
                        trigger: 'blur'
                    }
                ],
                new_password_confirm: [{
                        required: true,
                        message: '请再次密码',
                        trigger: 'blur'
                    },
                    {
                        validator: this.confirm_password_check,
                        trigger: 'blur',
                    },
                ],
            },
        };
    },
    methods: {
        change_password: function () {
            var vue_this = this;
            vue_this.$refs.user_input.validate((valid) => {
                if (valid) {
                    var shajs = require('sha.js')
                    var password_sha1 = shajs('sha1').update(vue_this.user_input.new_password).digest('hex');
                    vue_this.$call_remote_process("user_management", 'change_user_password', [vue_this.$cookies.get('zh_ssid'), password_sha1]).then(function (resp) {
                        if (resp) {
                            vue_this.$router.go(0);
                        }
                    });
                }
            });
        },
        confirm_password_check: function (rule, value, callback) {
            console.log(rule);
            if (value !== this.user_input.new_password) {
                callback(new Error('两次密码不一致'));
            } else {
                callback();
            }
        },
    },
}
</script>

<style>

</style>
