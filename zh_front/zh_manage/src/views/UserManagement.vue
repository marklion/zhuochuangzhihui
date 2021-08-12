<template>
<div class="user_management_show">
    <div class="block_title_show">权限说明</div>
    <el-table :data="all_permissions" style="width: 100%" stripe>
        <el-table-column prop="id" label="编号" min-width="20px">
        </el-table-column>
        <el-table-column prop="name" label="名称" min-width="50px">
        </el-table-column>
        <el-table-column prop="description" label="描述">
        </el-table-column>
    </el-table>
    <el-row type="flex" justify="space-between" align="middle">
        <el-col>
            <div class="block_title_show">所有用户</div>
        </el-col>
        <el-col>
            <div align="right">
                <el-button size="mini" type="success" icon="el-icon-plus" @click="show_add_user_diag = true;">新增</el-button>
            </div>
        </el-col>
    </el-row>
    <el-table :data="all_user" style="width: 100%" stripe>
        <el-table-column prop="id" label="编号" min-width="10px">
        </el-table-column>
        <el-table-column prop="name" label="姓名" min-width="25px">
        </el-table-column>
        <el-table-column prop="phone" label="电话" min-width="30px">
        </el-table-column>
        <el-table-column prop="permission_name" label="权限">
        </el-table-column>
        <el-table-column fixed="right" label="操作" min-width="50px">
            <template slot-scope="scope">
                <div v-if="scope.row.name != 'admin'">
                    <el-button type="warning" size="mini" @click="reset_user(scope.row)">重置</el-button>
                    <el-button @click="del_user(scope.row)" type="danger" size="mini">删除</el-button>
                </div>
            </template>
        </el-table-column>
    </el-table>
    <el-dialog title="新增用户" :visible.sync="show_add_user_diag" width="60%" @keyup.enter.native="add_user" @open="gen_random_password">
        <el-form :model="new_user" ref="add_user_form" :rules="rules" label-width="100px">
            <el-form-item label="姓名" prop="name">
                <el-input v-model="new_user.name" placeholder="请输入用户姓名"></el-input>
            </el-form-item>
            <el-form-item label="电话" prop="phone">
                <el-input v-model="new_user.phone" placeholder="请输入电话"></el-input>
            </el-form-item>
            <el-form-item label="权限" prop="permission_name">
                <el-select v-model="new_user.permission_name" placeholder="请分配权限">
                    <el-option v-for="(single_permission, index) in all_permissions" :key="index" :label="single_permission.name" :value="single_permission.name"></el-option>
                </el-select>
            </el-form-item>
            <el-form-item label="密码" prop="phone">
                <el-input v-model="new_user.password" disabled></el-input>
                <el-button type="text" size="mini" @click="copy_random_password(new_user.password)">复制密码</el-button>
                <span style="font-size:12px;">请复制密码，对话框关闭后将不再显示该密码</span>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="add_user">添加用户</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
    <el-dialog title="密码已重置" :visible.sync="show_user_reset_password_diag" width="30%" :before-close="clear_password">
        <div>{{user_reset_password}}</div>
        <div>
            <el-button type="text" size="mini" @click="copy_random_password(user_reset_password)">复制密码</el-button>
            <span style="font-size:12px;">请复制密码，对话框关闭后将不再显示该密码</span>
        </div>
    </el-dialog>
</div>
</template>

<script>
import Vue from 'vue'
import VueClipboard from 'vue-clipboard2'
Vue.use(VueClipboard)
export default {
    name: 'UserMangement',
    data: function () {
        return {
            all_permissions: [],
            all_user: [],
            show_add_user_diag: false,
            new_user: {
                name: '',
                phone: '',
                password: '',
                permission_name: '',
            },
            show_user_reset_password_diag: false,
            user_reset_password: '',
            rules: {
                name: [{
                    required: true,
                    message: '请输入姓名',
                    trigger: 'blur'
                }, ],
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
                permission_name: [{
                    required: true,
                    message: '请分配权限',
                    trigger: 'blur'
                }, ],
            },
        };
    },
    methods: {
        init_all_user: function () {
            var vue_this = this;
            vue_this.$call_remote_process("user_management", 'get_all_user', [vue_this.$cookies.get('zh_ssid')]).then(function (resp) {
                vue_this.all_user = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_user, index, element);
                });
            });
        },
        clear_password: function (done) {
            this.user_reset_password = "";
            done();
        },
        reset_user: function (_row) {
            var vue_this = this;
            var random_password = vue_this.make_random_passwod();
            var shajs = require('sha.js')
            var sha_random_password = shajs('sha1').update(random_password).digest('hex');
            vue_this.$alert("确定要重置 " + _row.name + ' 的密码吗？', '重置密码').then(function () {
                vue_this.$call_remote_process("user_management", 'reset_user', [vue_this.$cookies.get('zh_ssid'), _row.id, sha_random_password]).then(function (resp) {
                    if (resp) {
                        vue_this.show_user_reset_password_diag = true;
                        vue_this.user_reset_password = random_password;
                    }
                });
            });
        },
        del_user: function (_row) {
            var vue_this = this;
            vue_this.$alert('确定要删除 ' + _row.name + ' 吗？', '删除确认').then(function () {
                vue_this.$call_remote_process("user_management", 'del_user', [vue_this.$cookies.get('zh_ssid'), _row.id]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_user();
                    }
                });
            });
        },
        add_user: function () {
            var vue_this = this;
            this.$refs.add_user_form.validate((valid) => {
                if (valid) {
                    var shajs = require('sha.js')
                    vue_this.new_user.password = shajs('sha1').update(vue_this.new_user.password).digest('hex');
                    vue_this.$call_remote_process("user_management", 'add_user', [vue_this.$cookies.get('zh_ssid'), vue_this.new_user]).then(function (resp) {
                        if (resp) {
                            vue_this.init_all_user();
                            vue_this.show_add_user_diag = false;
                            vue_this.new_user = {
                                name: '',
                                phone: '',
                                password: '',
                                permission_name: '',
                            };
                        }
                    });
                }
            });
        },
        make_random_passwod: function () {
            var seed = ['A', 'B', 'C', 'D', 'E', 'F', 'G', 'H', 'I', 'J', 'K', 'L', 'M', 'N', 'P', 'Q', 'R', 'S', 'T', 'U', 'V', 'W', 'X', 'Y', 'Z', 'a', 'b', 'c', 'd', 'e', 'f', 'g', 'h', 'i', 'j', 'k', 'm', 'n', 'p', 'Q', 'r', 's', 't', 'u', 'v', 'w', 'x', 'y', 'z', '2', '3', '4', '5', '6', '7', '8', '9']; // 密码源数组
            var n = '';
            for (var i = 0; i < 8; i++) {
                var t = Math.round(Math.random() * (seed.length - 1));
                n += seed[t];
            }

            return n;
        },
        gen_random_password: function () {
            this.new_user.password = this.make_random_passwod();
        },
        copy_random_password: function (_password) {
            this.$copyText(_password);
            this.$message('密码已复制');
        },

    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("user_management", 'get_all_permissions', [vue_this.$cookies.get('zh_ssid')]).then(function (resp) {
            vue_this.all_permissions = [];
            resp.forEach((element, index) => {
                vue_this.$set(vue_this.all_permissions, index, element);
            });
        });
        vue_this.init_all_user();
    },
}
</script>

<style scoped>
.block_title_show {
    font-weight: bold;
    font-size: 20px;
    padding-top: 10px;
    padding-bottom: 10px;
    padding-left: 15px;
    color: rgb(88, 65, 0);
}
</style>
