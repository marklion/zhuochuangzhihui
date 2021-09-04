<template>
<div class="stuff_management_sh">
    <el-row type="flex" justify="space-between" align="middle">
        <el-col>
            <div class="block_title_show">所有物料</div>
        </el-col>
        <el-col>
            <div align="right" style="margin-right:10px;">
                <el-button size="mini" type="success" icon="el-icon-plus" @click="current_opt_add=true;show_edit_stuff_diag = true">新增</el-button>
            </div>
        </el-col>
    </el-row>
    <el-table :data="all_stuff" style="width: 100%" stripe>
        <el-table-column type="index" label="编号" min-width="10px">
        </el-table-column>
        <el-table-column prop="name" label="物料名" min-width="60px">
        </el-table-column>
        <el-table-column prop="unit" label="计量单位" min-width="30px">
        </el-table-column>
        <el-table-column prop="inventory" label="库存">
        </el-table-column>
        <el-table-column fixed="right" label="操作" min-width="50px">
            <template slot-scope="scope">
                <el-button type="warning" size="mini" @click="trigger_update_stuff(scope.row)">修改</el-button>
                <el-button type="danger" size="mini" @click="del_stuff(scope.row)">删除</el-button>
            </template>
        </el-table-column>
    </el-table>
    <el-dialog @close="clean_stuff" :title="(current_opt_add?'新增':'修改') + '物料'" :visible.sync="show_edit_stuff_diag" width="60%" @keyup.enter.native="edit_stuff">
        <el-form :model="focus_stuff" ref="edit_stuff_form" :rules="rules" label-width="120px">
            <el-form-item label="物料名称" prop="name">
                <el-input v-model="focus_stuff.name" placeholder="请输入物料名称"></el-input>
            </el-form-item>
            <el-form-item label="计量单位" prop="unit">
                <el-input v-model="focus_stuff.unit" placeholder="请输入计量单位"></el-input>
            </el-form-item>
            <el-form-item label="库存" prop="inventory">
                <el-input v-model="focus_stuff.inventory" type="number" placeholder="请输入库存"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="edit_stuff">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</div>
</template>

<script>
export default {
    name: 'StuffManagement',
    data: function () {
        return {
            current_opt_add: true,
            show_edit_stuff_diag: false,
            focus_stuff: {
                id: 0,
                name: '',
                unit: '',
                inventory: 0,
            },
            all_stuff: [],
            rules: {
                name: [{
                    required: true,
                    message: '请输入物料名称',
                    trigger: 'blur'
                }, ],
                unit: [{
                    required: true,
                    message: '请输入计量单位',
                    trigger: 'blur'
                }, ],
                inventory: [{
                    required: true,
                    message: '请输入库存',
                    trigger: 'blur'
                }, ],
            },
        };
    },
    methods: {
        del_stuff: function (_stuff) {
            var vue_this = this;
            this.$confirm('确定删除物料吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("stuff_management", "del_stuff", [vue_this.$cookies.get("zh_ssid"), _stuff.id]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_stuff();
                    }
                });
            });

        },
        trigger_update_stuff: function (_stuff) {
            var vue_this = this;
            vue_this.focus_stuff = {
                ..._stuff
            };
            vue_this.current_opt_add = false;
            vue_this.show_edit_stuff_diag = true;
        },
        edit_stuff: function () {
            var vue_this = this;
            var func_name = "add_stuff";
            if (!vue_this.current_opt_add) {
                func_name = "update_stuff";
            }
            vue_this.$refs.edit_stuff_form.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("stuff_management", func_name, [vue_this.$cookies.get("zh_ssid"), vue_this.focus_stuff]).then(function (resp) {
                        if (resp) {
                            vue_this.show_edit_stuff_diag = false;
                            vue_this.init_all_stuff();
                        }
                    });
                }
            });
        },
        init_all_stuff: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", 'get_all_stuff', [vue_this.$cookies.get('zh_ssid')]).then(function (resp) {
                vue_this.all_stuff = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_stuff, index, element);
                });
            });
        },
        clean_stuff: function () {
            this.focus_stuff = {
                id: 0,
                name: '',
                unit: '',
                inventory: 0,
            };
        },
    },
    beforeMount: function () {
        this.init_all_stuff();
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
