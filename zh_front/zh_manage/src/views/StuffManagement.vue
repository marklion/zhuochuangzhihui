<template>
<div class="stuff_management_sh">
    <el-row type="flex" justify="space-between" align="middle">
        <el-col>
            <div class="block_title_show">所有物料</div>
        </el-col>
        <el-col>
            <div style="margin-right:10px; text-align:right">
                <el-button size="mini" type="warning" icon="el-icon-search" @click="download_audit_log">下载审计日志</el-button>
                <table-import-export @proc_table="proc_upload_stuff" :sample_table="sample_table" export_name="物料导出表.xlsx" :export_table="all_stuff" :item_name_map="col_map"></table-import-export>
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
                <el-autocomplete v-model="focus_stuff.unit" :fetch-suggestions="fetch_unit" placeholder="请输入物料单位"></el-autocomplete>
            </el-form-item>
            <el-form-item label="库存" prop="inventory">
                <el-input-number v-model="focus_stuff.inventory" :min="0.001" label="请输入库存"></el-input-number>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="edit_stuff">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</div>
</template>

<script>
import TableImportExport from '../components/TableImportExport.vue'
import PinyinMatch from "pinyin-match"
export default {
    name: 'StuffManagement',
    components: {
        "table-import-export": TableImportExport,
    },
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
            col_map: {
                name: {
                    text: '物料名称'
                },
                unit: {
                    text: '计量单位'
                },
                inventory: {
                    text: '库存'
                },
            },
            sample_table: [{
                name: '中水',
                unit: '吨',
                inventory: '104'
            }],
        };
    },
    methods: {
        fetch_unit: function (_query, cb) {
            var ret = [{
                value: "吨"
            }, {
                value: "千克"
            }];
            if (_query) {
                var tmp_ret = ret;
                ret = [];
                tmp_ret.forEach(element => {
                    if (PinyinMatch.match(element.value, _query)) {
                        ret.push(element);
                    }
                });
            }
            cb(ret)
        },
        download_audit_log: function () {
            var FileSaver = require('file-saver');
            FileSaver.saveAs(this.$remote_file_url + '/audit_log.log', "审计日志.log");
        },
        proc_upload_stuff: async function (_array) {
            var vue_this = this;
            for (var i = 0; i < _array.length; i++) {
                try {
                    await vue_this.$call_remote_process("stuff_management", "add_stuff", [vue_this.$cookies.get("zh_ssid"), _array[i]]);
                } catch (err) {
                    console.log(err);
                }
                vue_this.init_all_stuff();
            }
        },
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

</style>
