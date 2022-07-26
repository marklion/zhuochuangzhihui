<template>
<div class="user_management_show">
    <el-row type="flex" justify="space-between" align="middle">
        <el-col>
            <div class="block_title_show">所有用户</div>
        </el-col>
        <el-col>
            <div align="right" style="margin-right:10px;">
                <el-button size="mini" type="success" icon="el-icon-plus" @click="show_add_user_diag = true;">新增</el-button>
            </div>
        </el-col>
    </el-row>
    <el-table :data="all_user" style="width: 100%" stripe>
        <el-table-column type="index" label="编号" min-width="10px">
        </el-table-column>
        <el-table-column prop="name" label="姓名" min-width="25px">
        </el-table-column>
        <el-table-column prop="phone" label="手机号" min-width="30px">
        </el-table-column>
        <el-table-column prop="permission_name" label="类型" sortable>
        </el-table-column>
        <el-table-column label="模块权限">
            <template slot-scope="scope">
                <el-tag v-for="(single_item, index) in scope.row.target_info" :key="index" effect="plain" :type="single_item.is_read?'success':'primary'" :closable="scope.row.name != 'admin'" @close=" del_permission_target(scope.row.id, single_item.id)">
                    {{single_item.description}}:{{single_item.is_read?'读取':'修改'}}
                </el-tag>
            </template>
        </el-table-column>
        <el-table-column fixed="right" label="操作" min-width="50px">
            <template slot-scope="scope">
                <div v-if="scope.row.name != 'admin'">
                    <el-button type="warning" size="mini" @click="reset_user(scope.row)">重置</el-button>
                    <el-button @click="del_user(scope.row)" type="danger" size="mini">删除</el-button>
                    <el-popover placement="bottom" width="400" trigger="click" v-if="scope.row.permission_name == '内部用户'">
                        <el-button size="mini" slot="reference" type="primary">增加模块</el-button>
                        <el-radio-group v-model="added_permission_target">
                            <el-radio v-for="(single_permission, index) in permisson_target_info" :key="index" :label="single_permission.target">{{single_permission.description}}</el-radio>
                        </el-radio-group>
                        <el-button size="mini" type="primary" @click="change_permisson_target(scope.row.id, true)">增加只读读权限</el-button>
                        <el-button size="mini" type="success" @click="change_permisson_target(scope.row.id, false)">增加权限</el-button>
                    </el-popover>
                </div>
            </template>
        </el-table-column>
    </el-table>
    <el-dialog title="新增用户" :visible.sync="show_add_user_diag" width="60%" @keyup.enter.native="add_user" @open="gen_random_password">
        <el-form :model="new_user" ref="add_user_form" :rules="rules" label-width="100px">
            <el-form-item label="姓名" prop="name">
                <el-input v-model="new_user.name" placeholder="请输入用户姓名"></el-input>
            </el-form-item>
            <el-form-item label="手机号" prop="phone">
                <el-input v-model="new_user.phone" placeholder="请输入手机号"></el-input>
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
            added_permission_target: '',
            permisson_target_info: [],
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
        change_permisson_target: function (_user_id, _is_read) {
            var vue_this = this;
            vue_this.$call_remote_process("user_management", 'add_user_permission_target', [vue_this.$cookies.get('zh_ssid'), _user_id, {
                target: vue_this.added_permission_target,
                is_read: true
            }]).then(function (resp) {
                if (resp) {
                    if (!_is_read) {
                        vue_this.$call_remote_process("user_management", 'add_user_permission_target', [vue_this.$cookies.get('zh_ssid'), _user_id, {
                            target: vue_this.added_permission_target,
                            is_read: false,
                        }]).then(function (resp) {
                            if (resp) {
                                vue_this.init_all_user();
                            }
                        })
                    } else {
                        vue_this.init_all_user();
                    }
                }
            });
        },
        init_all_user: function () {
            var vue_this = this;
            vue_this.$call_remote_process("user_management", 'get_all_user', [vue_this.$cookies.get('zh_ssid')]).then(function (resp) {
                vue_this.all_user = [];
                resp.forEach((element, index) => {
                    if (element.permission_name != '外部用户') {
                        element.permission_name = '内部用户';
                    }
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
                    vue_this.new_user.permission_name = '管理员';
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
        get_permission_target: function () {
            var vue_this = this;
            vue_this.$call_remote_process("user_management", "get_all_permission_item", []).then(function (resp) {
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.permisson_target_info, index, element);
                });
            });
        },
        del_permission_target: function (_user_id, _permission_id) {
            var vue_this = this;
            vue_this.$confirm('确定要取消该权限吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("user_management", "del_user_permission_target", [vue_this.$cookies.get("zh_ssid"), _user_id, _permission_id]).then(function () {
                    vue_this.init_all_user();
                })
            });
        }
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
        vue_this.get_permission_target();
    },
}
</script>

<style scoped>
</style>
