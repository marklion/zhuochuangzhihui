<template>
<div class="contract_management_show">
    <el-row type="flex" justify="space-between" align="middle">
        <el-col>
            <div class="block_title_show">所有合同</div>
        </el-col>
        <el-col>
            <div align="right" style="margin-right:10px;">
                <el-button size="mini" type="success" icon="el-icon-plus" @click="current_opt_add=true;show_add_contract_diag = true">新增</el-button>
            </div>
        </el-col>
    </el-row>
    <el-table :data="all_contract" style="width: 100%" stripe>
        <el-table-column type="index" label="编号" min-width="10px">
        </el-table-column>
        <el-table-column prop="name" label="公司名" min-width="60px">
        </el-table-column>
        <el-table-column prop="code" label="编码" min-width="25px">
        </el-table-column>
        <el-table-column label="类型" min-width="30px" :formatter="get_type">
        </el-table-column>
        <el-table-column prop="attachment" label="附件" min-width="45px">
            <template slot-scope="scope">
                <el-link v-if="scope.row.attachment" type="primary" :href="$remote_file_url + scope.row.attachment">查看</el-link>
                <span v-else>无附件</span>
            </template>
        </el-table-column>
        <el-table-column prop="date" label="修改日期">
        </el-table-column>
        <el-table-column fixed="right" label="操作" min-width="50px">
            <template slot-scope="scope">
                <el-button type="warning" size="mini" @click="trigger_update_contract(scope.row)">修改</el-button>
                <el-button type="danger" size="mini" @click="del_contract(scope.row)" >删除</el-button>
            </template>
        </el-table-column>
    </el-table>
    <el-dialog @close="clean_contract" :title="(current_opt_add?'新增':'修改') + '合同'" :visible.sync="show_add_contract_diag" width="60%" @keyup.enter.native="add_contract">
        <el-form :model="new_contract" ref="add_contract_form" :rules="rules" label-width="120px">
            <el-form-item label="对方公司名称" prop="name">
                <el-input v-model="new_contract.name" placeholder="请输入对方公司名称"></el-input>
            </el-form-item>
            <el-form-item label="合同类型" prop="is_sale">
                <el-radio-group v-model="new_contract.is_sale">
                    <el-radio :label="true">销售</el-radio>
                    <el-radio :label="false">采购</el-radio>
                </el-radio-group>
            </el-form-item>
            <el-form-item label="合同编号" prop="code">
                <el-input v-model="new_contract.code" placeholder="请输入合同编号(可选)"></el-input>
            </el-form-item>
            <el-form-item label="附件">
                <span v-if="!new_contract.attachment">未上传</span>
                <span v-else-if="new_contract.attachment.substring(0, 4) == '/tmp'">已上传</span>
                <el-row v-else type="flex" align="middle">
                    <el-col :span="6">
                        <el-link type="primary" :href="$remote_file_url + new_contract.attachment">附件</el-link>
                    </el-col>
                    <el-col :span="18">
                        <el-button size="small" type="danger" @click="new_contract.attachment = ''">删除附件</el-button>
                    </el-col>
                </el-row>
            </el-form-item>
            <el-form-item>
                <el-upload :on-remove="()=>{new_contract.attachment = ''}" accept="image/*,.pdf" :action="$remote_url + '/upload/'" :limit="1" :on-success="mark_tmp_path">
                    <el-button v-if="!new_contract.attachment" size="small" type="primary">点击上传</el-button>
                </el-upload>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="add_contract">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</div>
</template>

<script>
export default {
    name: 'ContractManagement',
    data: function () {
        return {
            current_opt_add: true,
            all_contract: [],
            show_add_contract_diag: false,
            new_contract: {
                name: '',
                code: '',
                attachment: '',
                is_sale: true,
            },
            rules: {
                name: [{
                    required: true,
                    message: '请输入公司名称',
                    trigger: 'blur'
                }, ],
                is_sale: [{
                    required: true,
                    message: '请选择合同类型',
                    trigger: 'blur'
                }],
            },
        };
    },
    methods: {
        clean_contract: function () {
            this.new_contract = {
                name: '',
                code: '',
                attachment: '',
                is_sale: true,
            };

        },
        trigger_update_contract: function (_contract) {
            var vue_this = this;
            vue_this.new_contract = {
                ..._contract
            };
            vue_this.current_opt_add = false;
            vue_this.show_add_contract_diag = true;
        },
        get_type: function (row) {
            return row.is_sale ? "销售" : "采购";
        },
        mark_tmp_path: function (resp, file) {
            var real_path = resp.match(/^\/tmp\/.*/gm)[0];
            var file_name_split = file.name.split('.');
            this.new_contract.attachment = real_path + '.' + file_name_split[file_name_split.length - 1];
        },
        del_contract: function (_contract) {
            var vue_this = this;
            this.$confirm('确定删除合同吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("contract_management", "del_contract", [vue_this.$cookies.get("zh_ssid"), _contract.id]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_contract();
                    }
                });
            });

        },
        add_contract: function () {
            var vue_this = this;
            var func_name = "add_contract";
            if (!vue_this.current_opt_add) {
                func_name = "update_contract";
            }
            vue_this.$refs.add_contract_form.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("contract_management", func_name, [vue_this.$cookies.get("zh_ssid"), vue_this.new_contract]).then(function (resp) {
                        if (resp) {
                            vue_this.show_add_contract_diag = false;
                            vue_this.init_all_contract();
                        }
                    });
                }
            });
        },
        init_all_contract: function () {
            var vue_this = this;
            vue_this.$call_remote_process("contract_management", "get_all_contract", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.all_contract = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_contract, index, element);
                });
            });
        },
    },
    beforeMount: function () {
        this.init_all_contract();
    },
}
</script>

<style scoped>
</style>
