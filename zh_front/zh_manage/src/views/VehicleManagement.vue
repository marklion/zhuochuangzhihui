<template>
<div class="vehicle_management_show">
    <el-row type="flex" justify="space-between" align="middle">
        <el-col>
            <div class="block_title_show">所有车辆</div>
        </el-col>
        <el-col>
            <div align="right" style="margin-right:10px;">
                <el-button size="mini" type="success" icon="el-icon-plus" @click="current_opt_add=true;show_edit_vehicle_diag = true">新增</el-button>
            </div>
        </el-col>
    </el-row>
    <el-table :data="all_vehicle" style="width: 100%" stripe>
        <el-table-column type="index" label="编号" min-width="10px">
        </el-table-column>
        <el-table-column prop="main_vehicle_number" label="主车" min-width="30px">
        </el-table-column>
        <el-table-column prop="behind_vehicle_number" label="挂车" min-width="30px">
        </el-table-column>
        <el-table-column prop="driver_name" label="司机姓名" min-width="30px">
        </el-table-column>
        <el-table-column prop="driver_phone" label="司机电话" min-width="40px">
        </el-table-column>
        <el-table-column prop="driver_id" label="司机身份证" min-width="60px">
        </el-table-column>
        <el-table-column sortable prop="group_name" label="分组名" min-width="30px">
        </el-table-column>
        <el-table-column sortable prop="company_name" label="所属公司" min-width="60px" :formatter="format_company_name">
        </el-table-column>
        <el-table-column fixed="right" label="操作" min-width="50px">
            <template slot-scope="scope">
                <el-button type="warning" size="mini" @click="trigger_update_vehicle(scope.row)">修改</el-button>
                <el-button type="danger" size="mini" @click="del_vehicle(scope.row)">删除</el-button>
            </template>
        </el-table-column>
    </el-table>
    <el-dialog @close="clean_vehicle" :title="(current_opt_add?'新增':'修改') + '车辆'" :visible.sync="show_edit_vehicle_diag" width="60%" @keyup.enter.native="edit_vehicle">
        <el-form :model="focus_vehicle" ref="focus_vehicle" :rules="rules" label-width="120px">
            <el-form-item label="所属公司" prop="group_name">
                <el-select v-model="focus_vehicle.company_name" placeholder="请选择所属公司">
                    <el-option v-for="(single_item,index) in company_for_select" :key="index" :label="single_item.label" :value="single_item.value"></el-option>
                </el-select>
            </el-form-item>
            <el-form-item label="主车车牌" prop="main_vehicle_number">
                <el-input v-model="focus_vehicle.main_vehicle_number" placeholder="请输入主车车牌"></el-input>
            </el-form-item>
            <el-form-item label="挂车车牌" prop="behind_vehicle_number">
                <el-input v-model="focus_vehicle.behind_vehicle_number" placeholder="请输入挂车车牌"></el-input>
            </el-form-item>
            <el-form-item label="司机姓名" prop="driver_name">
                <el-input v-model="focus_vehicle.driver_name" placeholder="请输入司机司机姓名"></el-input>
            </el-form-item>
            <el-form-item label="司机电话" prop="driver_phone">
                <el-input v-model="focus_vehicle.driver_phone" placeholder="请输入司机电话"></el-input>
            </el-form-item>
            <el-form-item label="司机身份证" prop="driver_id">
                <el-input v-model="focus_vehicle.driver_id" placeholder="请输入司机身份证"></el-input>
            </el-form-item>
            <el-form-item label="分组名" prop="group_name">
                <el-input v-model="focus_vehicle.group_name" placeholder="请输入分组名称"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="edit_vehicle">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</div>
</template>

<script>
export default {
    name: 'Vehicle_management',
    data: function () {
        var driver_info_complate = (rule, value, callback) => {
            var ret = false;
            console.log(this);
            console.log(rule, value);
            if (this.focus_vehicle.driver_name || this.focus_vehicle.driver_id || this.focus_vehicle.driver_phone) {
                ret = this.focus_vehicle.driver_name && this.focus_vehicle.driver_id && this.focus_vehicle.driver_phone;
            } else {
                ret = true;
            }

            if (!ret) {
                callback(new Error("请完整填写司机信息或全部留空"));
            } else {
                callback();
            }
        };
        return {
            company_for_select: [{
                value: '',
                label: '(自有)'
            }],
            current_opt_add: true,
            show_edit_vehicle_diag: false,
            focus_vehicle: {
                main_vehicle_number: '',
                behind_vehicle_number: '',
                driver_name: '',
                driver_phone: '',
                driver_id: '',
                company_name: '',
            },

            rules: {
                main_vehicle_number: [{
                    required: true,
                    message: '请输入主车车牌',
                    trigger: 'blur'
                }, {
                    pattern: /^(京[A-HJ-NPQY]|沪[A-HJ-N]|津[A-HJ-NPQR]|渝[A-DFGHN]|冀[A-HJRST]|晋[A-FHJ-M]|蒙[A-HJKLM]|辽[A-HJ-NP]|吉[A-HJK]|黑[A-HJ-NPR]|苏[A-HJ-N]|浙[A-HJKL]|皖[A-HJ-NP-S]|闽[A-HJK]|赣[A-HJKLMS]|鲁[A-HJ-NP-SUVWY]|豫[A-HJ-NP-SU]|鄂[A-HJ-NP-S]|湘[A-HJ-NSU]|粤[A-HJ-NP-Y]|桂[A-HJ-NPR]|琼[A-F]|川[A-HJ-MQ-Z]|贵[A-HJ]|云[AC-HJ-NP-SV]|藏[A-HJ]|陕[A-HJKV]|甘[A-HJ-NP]|青[A-H]|宁[A-E]|新[A-HJ-NP-S])([0-9A-HJ-NP-Z]{4}[0-9A-HJ-NP-Z挂试]|[0-9]{4}学|[A-D0-9][0-9]{3}警|[DF][0-9A-HJ-NP-Z][0-9]{4}|[0-9]{5}[DF])$|^WJ[京沪津渝冀晋蒙辽吉黑苏浙皖闽赣鲁豫鄂湘粤桂琼川贵云藏陕甘青宁新]?[0-9]{4}[0-9JBXTHSD]$|^(V[A-GKMORTV]|K[A-HJ-NORUZ]|H[A-GLOR]|[BCGJLNS][A-DKMNORVY]|G[JS])[0-9]{5}$|^[0-9]{6}使$|^([沪粤川渝辽云桂鄂湘陕藏黑]A|闽D|鲁B|蒙[AEH])[0-9]{4}领$|^粤Z[0-9A-HJ-NP-Z][0-9]{3}[港澳]$/i,
                    message: '请输入正确的车牌',
                    trigger: 'blur'
                }],
                behind_vehicle_number: [{
                    required: true,
                    message: '请输入挂车车牌',
                    trigger: 'blur'
                }, {
                    pattern: /^(京[A-HJ-NPQY]|沪[A-HJ-N]|津[A-HJ-NPQR]|渝[A-DFGHN]|冀[A-HJRST]|晋[A-FHJ-M]|蒙[A-HJKLM]|辽[A-HJ-NP]|吉[A-HJK]|黑[A-HJ-NPR]|苏[A-HJ-N]|浙[A-HJKL]|皖[A-HJ-NP-S]|闽[A-HJK]|赣[A-HJKLMS]|鲁[A-HJ-NP-SUVWY]|豫[A-HJ-NP-SU]|鄂[A-HJ-NP-S]|湘[A-HJ-NSU]|粤[A-HJ-NP-Y]|桂[A-HJ-NPR]|琼[A-F]|川[A-HJ-MQ-Z]|贵[A-HJ]|云[AC-HJ-NP-SV]|藏[A-HJ]|陕[A-HJKV]|甘[A-HJ-NP]|青[A-H]|宁[A-E]|新[A-HJ-NP-S])([0-9A-HJ-NP-Z]{4}[0-9A-HJ-NP-Z挂试]|[0-9]{4}学|[A-D0-9][0-9]{3}警|[DF][0-9A-HJ-NP-Z][0-9]{4}|[0-9]{5}[DF])$|^WJ[京沪津渝冀晋蒙辽吉黑苏浙皖闽赣鲁豫鄂湘粤桂琼川贵云藏陕甘青宁新]?[0-9]{4}[0-9JBXTHSD]$|^(V[A-GKMORTV]|K[A-HJ-NORUZ]|H[A-GLOR]|[BCGJLNS][A-DKMNORVY]|G[JS])[0-9]{5}$|^[0-9]{6}使$|^([沪粤川渝辽云桂鄂湘陕藏黑]A|闽D|鲁B|蒙[AEH])[0-9]{4}领$|^粤Z[0-9A-HJ-NP-Z][0-9]{3}[港澳]$/i,
                    message: '请输入正确的车牌',
                    trigger: 'blur'
                }, {
                    validator: (rule, value, callback) => {
                        console.log(rule);
                        if (value.substr(value.length - 1, value.length) != "挂") {
                            callback(new Error("挂车牌结尾必须为挂"));
                        } else {
                            callback();
                        }
                    },
                    trigger: 'blur',
                }],
                driver_phone: [{
                    pattern: /^(13[0-9]|14[01456879]|15[0-35-9]|16[2567]|17[0-8]|18[0-9]|19[0-35-9])\d{8}$/,
                    message: '请输入正确手机号',
                    trigger: 'blur'
                }, {
                    validator: driver_info_complate,
                    trigger: 'blur'
                }],
                driver_name: [{
                    validator: driver_info_complate,
                    trigger: 'blur'
                }],
                driver_id: [{
                    pattern: /^[1-9]\d{5}(18|19|20|(3\d))\d{2}((0[1-9])|(1[0-2]))(([0-2][1-9])|10|20|30|31)\d{3}[0-9Xx]$/,
                    message: '请输入正确身份证号',
                    trigger: 'blur'
                }, {
                    validator: driver_info_complate,
                    trigger: 'blur'
                }],
            },
            all_vehicle: [],
            vehicle_based_on_company: function (company_name) {
                var ret = [];
                var vue_this = this;
                vue_this.all_vehicle.forEach(element => {
                    if (element.company_name == company_name) {
                        ret.push(element);
                    }
                });

                return ret;
            },
        };
    },
    methods: {
        format_company_name: function (value) {
            if (value.company_name) {
                return value.company_name;
            } else {
                return "(自有)"
            }
        },
        del_vehicle: function (vehicle) {
            var vue_this = this;
            this.$confirm('确定删除车辆吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("vehicle_management", "del_vehicle", [vue_this.$cookies.get("zh_ssid"), vehicle.id]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_vehicle();
                    }
                });
            });
        },
        trigger_update_vehicle: function (vehicle) {
            var vue_this = this;
            vue_this.focus_vehicle = {
                ...vehicle
            };
            vue_this.show_edit_vehicle_diag = true;
            vue_this.current_opt_add = false;
        },
        init_all_vehicle: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_management", "get_all_vehicle", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.all_vehicle = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_vehicle, index, element);
                });
            });
        },
        clean_vehicle: function () {
            this.focus_vehicle = {
                main_vehicle_number: '',
                behind_vehicle_number: '',
                driver_name: '',
                driver_phone: '',
                driver_id: '',
                company_name: '',
            };
        },
        edit_vehicle: function () {
            console.log(this.focus_vehicle);
            var vue_this = this;
            var func_name = "add_vehicle";
            if (!vue_this.current_opt_add) {
                func_name = "update_vehicle";
            }
            vue_this.focus_vehicle.main_vehicle_number = vue_this.focus_vehicle.main_vehicle_number.toUpperCase();
            vue_this.focus_vehicle.behind_vehicle_number = vue_this.focus_vehicle.behind_vehicle_number.toUpperCase();
            vue_this.$refs.focus_vehicle.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("vehicle_management", func_name, [vue_this.$cookies.get("zh_ssid"), vue_this.focus_vehicle]).then(function (resp) {
                        if (resp) {
                            vue_this.init_all_vehicle();
                            vue_this.show_edit_vehicle_diag = false;
                        }
                    });
                }
            });
        },
    },
    beforeMount: function () {
        this.init_all_vehicle();
        var vue_this = this;
        vue_this.$call_remote_process("contract_management", 'get_all_contract', [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
            resp.forEach(element => {
                vue_this.company_for_select.push({
                    value: element.name,
                    label: element.name,
                });
            });
        });
    }
}
</script>

<style scoped>
</style>
