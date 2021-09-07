<template>
<div id="app">
    <div class="web_header_show">
        <el-row type="flex" justify="space-between" align="middle">
            <el-col>
                <div class="web_header_content_show">
                    卓创智汇
                </div>
            </el-col>
            <el-col :span="12">
                <div align="right" class="user_profile_show">
                    <el-button v-if="!$store.state.is_login" @click="show_login_diag = true">登录</el-button>
                    <div v-else>
                        <div class="user_info_show">
                            <div>当前用户：{{$store.state.user_info.name}}</div>
                            <div>权限：{{$store.state.user_info.permission_name}}</div>
                        </div>
                        <el-button type="warning" size="mini" @click="show_change_password = true">修改密码</el-button>
                        <el-button type="danger" size="mini" @click="user_logoff">退出登录</el-button>
                    </div>
                </div>
            </el-col>
        </el-row>
    </div>
    <div>
        <el-row>
            <el-col :span="4">
                <el-menu class="web_nav_show" default-active="Home" router background-color="#545c64" text-color="#fff">
                    <el-menu-item v-if="$store.state.user_info.permission <= 2" index="Home" :route="{name:'Home'}">概览</el-menu-item>
                    <el-menu-item v-if="$store.state.user_info.permission <= 2" index="ContractManagement" :route="{name:'ContractManagement'}">合同管理</el-menu-item>
                    <el-menu-item v-if="$store.state.user_info.permission <= 2" index="VehicleManagement" :route="{name:'VehicleManagement'}">车辆管理</el-menu-item>
                    <el-menu-item v-if="$store.state.user_info.permission <= 2" index="StuffManagement" :route="{name:'StuffManagement'}">物料管理</el-menu-item>
                    <el-menu-item v-if="$store.state.user_info.permission <= 0" index="UserManagement" :route="{name:'UserManagement'}">用户管理</el-menu-item>
                    <el-menu-item v-if="$store.state.user_info.permission <= 0" index="SystemManagement" :route="{name:'SystemManagement'}">系统设置</el-menu-item>
                </el-menu>
            </el-col>
            <el-col :span="20">
                <div v-if="$store.state.is_login" style="height:660px;">
                    <router-view></router-view>
                </div>
                <div v-else>
                    请先登录
                </div>
            </el-col>
        </el-row>
    </div>
    <el-dialog title="登录" :visible.sync="show_login_diag" width="40%" @keyup.enter.native="user_login">
        <el-form :model="login_form" :rules="rules" label-width="100px">
            <el-form-item label="手机号" prop="phone">
                <el-input v-model="login_form.phone" placeholder="请输入手机号"></el-input>
            </el-form-item>
            <el-form-item label="密码" prop="password">
                <el-input v-model="login_form.password" show-password placeholder="请输入密码"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="user_login">登陆</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
    <el-dialog title="修改密码" :close-on-click-modal="false" :close-on-press-escape="false" :visible.sync="need_show_change_password" width="60%">
        <change-password></change-password>
    </el-dialog>
</div>
</template>

<script>
import ChangePassword from './components/ChangePassword.vue'
export default {
    name: 'App',
    components: {
        "change-password": ChangePassword,
    },
    computed: {
        need_show_change_password: {
            get() {
                return this.$store.state.user_info.need_change_password || this.show_change_password;
            },
            set(_value) {
                this.show_change_password = _value;
            }
        },
    },
    data: function () {
        return {
            login_form: {
                phone: '',
                password: '',
            },
            show_change_password: false,
            show_login_diag: false,
            rules: {
                phone: [{
                        required: true,
                        message: '请输入手机号',
                        trigger: 'blur'
                    },
                    {
                        pattern: /^(13[0-9]|14[01456879]|15[0-35-9]|16[2567]|17[0-8]|18[0-9]|19[0-35-9])\d{8}$/,
                        message: '请输入正确手机号',
                        trigger: 'blur'
                    }
                ],
                password: [{
                    required: true,
                    message: '请输入密码',
                    trigger: 'blur'
                }, ],
            },
        };
    },
    methods: {
        user_login: function () {
            var vue_this = this;
            var shajs = require('sha.js')
            var password_sha1 = shajs('sha1').update(vue_this.login_form.password).digest('hex');
            vue_this.$call_remote_process("user_management", "user_login", [vue_this.login_form.phone, password_sha1]).then(function (resp) {
                if (resp.length > 0) {
                    vue_this.$cookies.set('zh_ssid', resp, '5d');
                    vue_this.show_login_diag = false;
                    vue_this.init_user_info();
                }
            });
        },
        init_user_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("user_management", 'self_info', [vue_this.$cookies.get('zh_ssid')]).then(function (resp) {
                if (resp.id != 0) {
                    vue_this.$store.commit('set_user_info', resp);
                    vue_this.$store.commit('set_login', true);
                } else {
                    vue_this.$store.commit('set_user_info', {
                        name: '',
                        permission: -1,
                        id: 0,
                        permission_name: '',
                        phone: '',
                        need_change_password: false,
                    });
                    vue_this.$store.commit('set_login', false);
                }
            });
        },
        user_logoff: function () {
            var vue_this = this;
            vue_this.$call_remote_process("user_management", 'user_logoff', [vue_this.$cookies.get('zh_ssid')]).then(function () {
                vue_this.$router.go(0);
            });
        },
    },
    beforeMount: function () {
        this.init_user_info();
    },
}
</script>

<style scoped>
.web_header_content_show {
    padding-left: 40px;
    color: bisque;
    font-weight: bold;
    font-size: 40px;
    line-height: 80px;
}

.web_header_show {
    background-color: rgb(114, 57, 20);
}

.web_nav_show {
    height: 660px;
}

.user_profile_show {
    padding-right: 20px;
}

.user_info_show {
    color: white;
}
</style>
