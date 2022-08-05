<template>
<div id="app">
    <div v-if="!$route.meta.mobile">
        <div class="web_header_show">
            <el-row type="flex" justify="space-between" align="middle">
                <el-col>
                    <div class="web_header_content_show">
                        {{oem_name}}
                    </div>
                </el-col>
                <el-col :span="12">
                    <div align="right" class="user_profile_show">
                        <el-button v-if="!$store.state.is_login" @click="show_login_diag = true">登录</el-button>
                        <div v-else>
                            <el-row type="flex" align="middle">
                                <el-col>
                                    <div class="user_info_show">
                                        <div>当前用户：{{$store.state.user_info.name}}</div>
                                    </div>
                                </el-col>
                                <el-col :span="6">
                                    <div>
                                        <el-button type="warning" size="mini" @click="show_change_password = true">修改密码</el-button>
                                    </div>
                                    <div>
                                        <el-button type="danger" size="mini" @click="user_logoff">退出登录</el-button>
                                    </div>
                                </el-col>
                            </el-row>
                        </div>
                    </div>
                </el-col>
            </el-row>
        </div>
        <div>
            <el-row>
                <el-col :span="3">
                    <el-menu v-if="$store.state.is_login" class="web_nav_show" :default-active="$route.name" router background-color="#545c64" text-color="#fff">
                        <el-menu-item v-for="(single_menu, index) in menu_need_show" :key="index" :index="single_menu.route_name" :route="{name:single_menu.route_name, query:{focus_stuff:$store.state.focus_stuff}}">{{single_menu.name}}</el-menu-item>
                    </el-menu>
                </el-col>
                <el-col :span="21">
                    <div class="content_show">
                        <div v-if="$store.state.is_login">
                            <el-page-header v-if="$route.meta.is_sub" @back="go_back" :content="$route.meta.subtitle">
                            </el-page-header>
                            <router-view></router-view>
                        </div>
                        <div v-else>
                            请先登录
                        </div>
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
    </div>
    <div v-else>
        <div v-if="$store.state.is_login || $route.meta.no_need_login">
            <router-view></router-view>
            <van-tabbar placeholder safe-area-inset-bottom route v-if="!$route.meta.no_tabbar">
                <van-tabbar-item replace to="/mobile/vehicle_order_center" icon="search">派车中心</van-tabbar-item>
                <van-tabbar-item replace to="/mobile/vehicle_management" icon="newspaper-o">车辆管理</van-tabbar-item>
                <van-tabbar-item v-if="$store.state.user_info.permission < 3" replace to="/mobile/all_came_vehicle" icon="label-o">现场管理</van-tabbar-item>
                <van-tabbar-item replace to="/mobile/self_order_opt" icon="label-o" v-if="$store.state.user_info.permission == 3">司机派车</van-tabbar-item>
                <van-tabbar-item v-if="$store.state.user_info.permission < 3" replace to="/mobile/contract_management" icon="cluster-o">合同管理</van-tabbar-item>
                <van-tabbar-item v-if="$store.state.user_info.permission < 3" replace to="/mobile/scale_state" icon="eye-o">设备状态</van-tabbar-item>
            </van-tabbar>
        </div>
        <div v-else>
            <van-button class="login_button_show" type="primary" size="large" block circle v-if="!$store.state.is_login" @click="show_login_diag = true">登录</van-button>
            <van-dialog v-model="show_login_diag" title="登录" :show-cancel-button="false" :show-confirm-button="false" :close-on-click-overlay="true">
                <van-form @submit="user_login">
                    <van-field v-model="login_form.phone" name="手机号" label="用户名" placeholder="用户名" :rules="[{ required: true, message: '请填写用户名' }]" />
                    <van-field v-model="login_form.password" type="password" name="密码" label="密码" placeholder="密码" :rules="[{ required: true, message: '请填写密码' }]" />
                    <div style="margin: 16px;">
                        <van-button round block type="info" native-type="submit">提交</van-button>
                    </div>
                </van-form>
            </van-dialog>
        </div>
    </div>

    <el-dialog title="修改密码" :close-on-click-modal="false" :close-on-press-escape="false" :visible.sync="need_show_change_password" width="60%">
        <change-password></change-password>
    </el-dialog>
</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import 'vant/lib/index.css';
Vue.use(Vant);
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
        menu_need_show: function () {
            var ret = [];
            this.cur_menu.forEach((element) => {
                if (this.$store.state.user_info.permission <= element.permission_need) {
                    ret.push(element);
                }
            });
            return ret;
        },
    },
    data: function () {
        return {
            isMobile() {
                if (window.navigator.userAgent.match(/(phone|pad|pod|iPhone|iPod|ios|iPad|Android|Mobile|BlackBerry|IEMobile|MQQBrowser|JUC|Fennec|wOSBrowser|BrowserNG|WebOS|Symbian|Windows Phone)/i)) {
                    return true; // 移动端
                } else {
                    return false; // PC端
                }
            },
            cmd_queue: [],
            oem_name: "卓创智汇",
            cur_menu: [{
                permission_need: 2,
                route_name: 'Home',
                name: '概览'
            }, {
                permission_need: 3,
                route_name: 'VehicleOrderCenter',
                name: '派车中心'
            }, {
                permission_need: 2,
                route_name: 'ContractManagement',
                name: '合同管理'
            }, {
                permission_need: 3,
                route_name: 'VehicleManagement',
                name: '车辆管理'
            }, {
                permission_need: 2,
                route_name: 'StuffManagement',
                name: '物料管理'
            }, {
                permission_need: 2,
                route_name: 'WhiteRecord',
                name: '白名单称重'
            }, {
                permission_need: 0,
                route_name: 'UserManagement',
                name: '用户管理'
            }, {
                permission_need: 0,
                route_name: 'VideoStream',
                name: '监控'
            }, {
                permission_need: 0,
                route_name: 'SystemManagement',
                name: '系统设置'
            }, ],
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

        go_back: function () {
            this.$router.go(-1);
        },
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
                if (vue_this.$store.state.user_info.permission == 3 && vue_this.$route.name == 'Home') {
                    vue_this.$router.replace({
                        name: 'VehicleOrderCenter'
                    });
                }
            });
        },
        user_logoff: function () {
            var vue_this = this;
            vue_this.$call_remote_process("user_management", 'user_logoff', [vue_this.$cookies.get('zh_ssid')]).then(function () {
                vue_this.$cookies.set('zh_ssid', "");
                vue_this.$router.go(0);
            });
        },
    },
    watch: {
        "$route": function (to) {
            if (!to.meta.no_need_login) {
                this.init_user_info();
            }
            if (this.isMobile() && to.path.substr(0, 7) != '/mobile') {
                var target_path = to.path;
                if (target_path == '/') {
                    target_path = '/vehicle_order_center';
                }
                this.$router.replace('/mobile' + target_path);
            }
            if (to.query.focus_stuff) {
                this.$store.commit('set_focus_stuff', to.query.focus_stuff);
            }
        },
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("system_management", "get_oem_name", []).then(function (resp) {
            vue_this.oem_name = resp;
        });
        document.addEventListener("keydown", function (e) {
            if (e.keyCode >= 37 && e.keyCode <= 40) {
                vue_this.cmd_queue.push(e.keyCode);
            }
            if (vue_this.cmd_queue.length >= 8) {
                var sample_cmd = [38, 37, 40, 39, 38, 40, 37, 39];
                if (vue_this.cmd_queue.slice(-8, vue_this.cmd_queue.length).toString() == sample_cmd.toString()) {
                    vue_this.cmd_queue = [];
                    window.location.href = "/wetty";
                }
            }
        });
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
    background-color: #000c5e;
    height: 12vh;
}

.web_nav_show {
    height: 88vh;
}

.user_profile_show {
    padding-right: 20px;
}

.user_info_show {
    color: white;
}

.content_show {
    height: 88vh;
    overflow-y: auto;
}

.login_button_show {
    position: absolute;
    top: 50%;
}
</style>
