<template>
<div class="vehicle_management_show">
    <div v-if="!$route.meta.mobile">

        <el-row type="flex" justify="space-between" align="middle">
            <el-col>
                <div class="block_title_show">所有车辆</div>
            </el-col>
            <el-col>
                <div align="right" style="margin-right:10px;">
                    <el-input v-model="search_key" placeholder="输入部分车号过滤" prefix-icon="el-icon-search"></el-input>
                </div>
            </el-col>
            <el-col>
                <div style="margin-right:10px; text-align:right">
                    <table-import-export @proc_table="proc_upload_vehicle" :sample_table="sample_table()" export_name="车辆导出表.xlsx" :export_table="all_vehicle" :item_name_map="col_map()"></table-import-export>
                    <el-button size="mini" type="success" icon="el-icon-plus" @click="current_opt_add=true;show_edit_vehicle_diag = true">新增</el-button>
                </div>
            </el-col>
        </el-row>
        <el-table :data="vehicle_need_show" style="width: 100%" stripe>
            <el-table-column type="index" label="编号" width="50px">
            </el-table-column>
            <el-table-column prop="main_vehicle_number" label="主车" width="100px">
            </el-table-column>
            <el-table-column prop="behind_vehicle_number" label="挂车" width="100px">
            </el-table-column>
            <el-table-column prop="driver_name" label="司机姓名" width="80px">
            </el-table-column>
            <el-table-column prop="driver_phone" label="司机电话" width="120px">
            </el-table-column>
            <el-table-column prop="driver_id" label="司机身份证" width="180px">
            </el-table-column>
            <el-table-column sortable prop="group_name" label="分组名" width="90px">
            </el-table-column>
            <el-table-column sortable prop="company_name" label="所属公司" width="200px" :formatter="format_company_name">
            </el-table-column>
            <el-table-column prop="in_white_list" label="加入白名单" width="100px" :formatter="format_white">
            </el-table-column>
            <el-table-column prop="in_black_list" label="加入黑名单" width="100px" :formatter="format_black">
            </el-table-column>
            <el-table-column prop="max_count" label="最大净重" width="100px">
            </el-table-column>
            <el-table-column fixed="right" label="操作" width="150px">
                <template slot-scope="scope">
                    <el-button type="warning" size="mini" @click="trigger_update_vehicle(scope.row)">修改</el-button>
                    <el-button type="danger" size="mini" @click="del_vehicle(scope.row)">删除</el-button>
                </template>
            </el-table-column>
        </el-table>
        <el-dialog @close="clean_vehicle" :title="(current_opt_add?'新增':'修改') + '车辆'" :visible.sync="show_edit_vehicle_diag" width="60%" @keyup.enter.native="edit_vehicle">
            <el-form :model="focus_vehicle" ref="focus_vehicle" :rules="rules" label-width="120px">
                <el-form-item label="所属公司" prop="company_name">
                    <el-select v-model="focus_vehicle.company_name" placeholder="请选择所属公司" :disabled="$store.state.user_info.permission == 3">
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
                <el-form-item label="最大净重" prop="max_count">
                    <el-input v-model="focus_vehicle.max_count" type="number" placeholder="最大净重"></el-input>
                </el-form-item>
                <el-form-item v-if="$store.state.user_info.permission != 3 && focus_vehicle.company_name == ''" label="添加白名单" prop="in_white_list">
                    <el-switch v-model="focus_vehicle.in_white_list" active-color="#13ce66" inactive-color="#ff4949">
                    </el-switch>
                </el-form-item>
                <el-form-item v-if="$store.state.user_info.permission != 3 && focus_vehicle.company_name == ''" label="添加黑名单" prop="in_black_list">
                    <el-switch v-model="focus_vehicle.in_black_list" active-color="#13ce66" inactive-color="#ff4949">
                    </el-switch>
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
    <div v-else>
        <van-pull-refresh v-model="isLoading" @refresh="onRefresh">
            <van-nav-bar title="车辆管理" right-text="新增" @click-right="current_opt_add=true;show_edit_vehicle_diag = true" />
            <van-search v-model="search_key" placeholder="车牌号过滤" />
            <van-cell v-for="(single_vehicle, index) in vehicle_need_show" :key="index" center :title="single_vehicle.main_vehicle_number + '-' + single_vehicle.behind_vehicle_number" :label="single_vehicle.company_name">
                <template #right-icon>
                    <van-button size="mini" type="info" @click="trigger_update_vehicle(single_vehicle)">修改</van-button>
                    <van-button size="mini" type="danger" @click="del_vehicle(single_vehicle)">删除</van-button>
                </template>
                <span>{{single_vehicle.driver_name}}</span>
                <span>{{single_vehicle.driver_phone}}</span>
                <div>{{single_vehicle.driver_id}}</div>
            </van-cell>
        </van-pull-refresh>
        <van-dialog v-model="show_edit_vehicle_diag" @close="clean_vehicle" :title="(current_opt_add?'新增':'修改') + '车辆'" :showConfirmButton="false" closeOnClickOverlay>
            <van-form @submit="edit_vehicle">
                <van-field required v-model="focus_vehicle.main_vehicle_number" name="主车车牌" label="主车车牌" placeholder="主车车牌" :rules="[{ required: true, message: '请填写主车车牌' }, {
                    pattern: /^(京[A-HJ-NPQY]|沪[A-HJ-N]|津[A-HJ-NPQR]|渝[A-DFGHN]|冀[A-HJRST]|晋[A-FHJ-M]|蒙[A-HJKLM]|辽[A-HJ-NP]|吉[A-HJK]|黑[A-HJ-NPR]|苏[A-HJ-N]|浙[A-HJKL]|皖[A-HJ-NP-S]|闽[A-HJK]|赣[A-HJKLMS]|鲁[A-HJ-NP-SUVWY]|豫[A-HJ-NP-SU]|鄂[A-HJ-NP-S]|湘[A-HJ-NSU]|粤[A-HJ-NP-Y]|桂[A-HJ-NPR]|琼[A-F]|川[A-HJ-MQ-Z]|贵[A-HJ]|云[AC-HJ-NP-SV]|藏[A-HJ]|陕[A-HJKV]|甘[A-HJ-NP]|青[A-H]|宁[A-E]|新[A-HJ-NP-S])([0-9A-HJ-NP-Z]{4}[0-9A-HJ-NP-Z挂试]|[0-9]{4}学|[A-D0-9][0-9]{3}警|[DF][0-9A-HJ-NP-Z][0-9]{4}|[0-9]{5}[DF])$|^WJ[京沪津渝冀晋蒙辽吉黑苏浙皖闽赣鲁豫鄂湘粤桂琼川贵云藏陕甘青宁新]?[0-9]{4}[0-9JBXTHSD]$|^(V[A-GKMORTV]|K[A-HJ-NORUZ]|H[A-GLOR]|[BCGJLNS][A-DKMNORVY]|G[JS])[0-9]{5}$|^[0-9]{6}使$|^([沪粤川渝辽云桂鄂湘陕藏黑]A|闽D|鲁B|蒙[AEH])[0-9]{4}领$|^粤Z[0-9A-HJ-NP-Z][0-9]{3}[港澳]$/i,
                    message: '请输入正确的车牌',
                }]" />
                <van-field v-model="focus_vehicle.behind_vehicle_number" name="挂车车牌" label="挂车车牌" placeholder="挂车车牌" />
                <van-field required v-model="focus_vehicle.driver_name" name="司机姓名" label="司机姓名" placeholder="司机姓名" />
                <van-field required v-model="focus_vehicle.driver_phone" name="司机电话" label="司机电话" placeholder="司机电话" :rules="[{
                    pattern: /^(13[0-9]|14[01456879]|15[0-35-9]|16[2567]|17[0-8]|18[0-9]|19[0-35-9])\d{8}$/,
                    message: '请输入正确手机号',
                }]" />
                <van-field required v-model="focus_vehicle.driver_id" name="司机身份证号" label="司机身份证号" placeholder="司机身份证号" :rules="[{
                    pattern: /^[1-9]\d{5}(18|19|20|(3\d))\d{2}((0[1-9])|(1[0-2]))(([0-2][1-9])|10|20|30|31)\d{3}[0-9Xx]$/,
                    message: '请输入正确身份证号',
                    trigger: 'blur'
                }]" />
                <div style="margin: 16px;">
                    <van-button round block type="info" native-type="submit">提交</van-button>
                </div>
            </van-form>
        </van-dialog>
    </div>
</div>
</template>

<script>
import TableImportExport from '../components/TableImportExport.vue'
import PinyinMatch from "pinyin-match"
export default {
    name: 'Vehicle_management',
    components: {
        "table-import-export": TableImportExport,
    },
    data: function () {
        return {
            search_key: '',
            isLoading: false,
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
                in_white_list: false,
                group_name: "",
                max_count: 35,
            },
            sample_table: function () {
                var ret = {
                    main_vehicle_number: '豫A33421',
                    behind_vehicle_number: '川B3321挂',
                    driver_name: '张三',
                    driver_phone: '13987654321',
                    driver_id: '110110201010101010',
                    company_name: '采购公司1',
                    group_name: "购煤组",
                    max_count: 35,
                };
                var ret2 = {
                    main_vehicle_number: '新A38821',
                    behind_vehicle_number: '皖B3561挂',
                    driver_name: '李四',
                    driver_phone: '17788996655',
                    driver_id: '150150201505050505',
                    company_name: '(自有)',
                    group_name: "送灰组",
                    max_count: 35,
                };
                if (this.$store.state.user_info.permission == 3) {
                    ret.company_name = this.$store.state.user_info.name;
                    ret2.company_name = this.$store.state.user_info.name;
                }
                return [ret, ret2];
            },
            col_map: function () {
                var ret = {
                    main_vehicle_number: {
                        text: '主车车号'
                    },
                    behind_vehicle_number: {
                        text: '挂车车号'
                    },
                    driver_name: {
                        text: '司机姓名'
                    },
                    driver_phone: {
                        text: '司机电话'
                    },
                    driver_id: {
                        text: '司机身份证号'
                    },
                    company_name: {
                        text: '所属公司'
                    },
                    group_name: {
                        text: '分组名'
                    },
                    max_count: {
                        text: '最大净重'
                    },
                }
                return ret;
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
                    required: false,
                    message: '请输入挂车车牌',
                    trigger: 'blur'
                }, {
                    pattern: /^(京[A-HJ-NPQY]|沪[A-HJ-N]|津[A-HJ-NPQR]|渝[A-DFGHN]|冀[A-HJRST]|晋[A-FHJ-M]|蒙[A-HJKLM]|辽[A-HJ-NP]|吉[A-HJK]|黑[A-HJ-NPR]|苏[A-HJ-N]|浙[A-HJKL]|皖[A-HJ-NP-S]|闽[A-HJK]|赣[A-HJKLMS]|鲁[A-HJ-NP-SUVWY]|豫[A-HJ-NP-SU]|鄂[A-HJ-NP-S]|湘[A-HJ-NSU]|粤[A-HJ-NP-Y]|桂[A-HJ-NPR]|琼[A-F]|川[A-HJ-MQ-Z]|贵[A-HJ]|云[AC-HJ-NP-SV]|藏[A-HJ]|陕[A-HJKV]|甘[A-HJ-NP]|青[A-H]|宁[A-E]|新[A-HJ-NP-S])([0-9A-HJ-NP-Z]{4}[0-9A-HJ-NP-Z挂试]|[0-9]{4}学|[A-D0-9][0-9]{3}警|[DF][0-9A-HJ-NP-Z][0-9]{4}|[0-9]{5}[DF])$|^WJ[京沪津渝冀晋蒙辽吉黑苏浙皖闽赣鲁豫鄂湘粤桂琼川贵云藏陕甘青宁新]?[0-9]{4}[0-9JBXTHSD]$|^(V[A-GKMORTV]|K[A-HJ-NORUZ]|H[A-GLOR]|[BCGJLNS][A-DKMNORVY]|G[JS])[0-9]{5}$|^[0-9]{6}使$|^([沪粤川渝辽云桂鄂湘陕藏黑]A|闽D|鲁B|蒙[AEH])[0-9]{4}领$|^粤Z[0-9A-HJ-NP-Z][0-9]{3}[港澳]$/i,
                    message: '请输入正确的车牌',
                    trigger: 'blur'
                }, {
                    validator: (rule, value, callback) => {
                        console.log(rule);
                        if (value.length > 0 && value.substr(value.length - 1, value.length) != "挂") {
                            callback(new Error("挂车牌结尾必须为挂"));
                        } else {
                            callback();
                        }
                    },
                    trigger: 'blur',
                }],
                driver_phone: [{
                    required: true,
                    message: '请输入司机电话',
                    trigger: 'blur'
                }, {
                    pattern: /^(13[0-9]|14[01456879]|15[0-35-9]|16[2567]|17[0-8]|18[0-9]|19[0-35-9])\d{8}$/,
                    message: '请输入正确手机号',
                    trigger: 'blur'
                }],
                driver_name: [{
                    required: true,
                    message: '请输入司机姓名',
                    trigger: 'blur'
                }],
                driver_id: [{
                    required: true,
                    message: '请输入司机身份证',
                    trigger: 'blur'
                }, {
                    pattern: /^[1-9]\d{5}(18|19|20|(3\d))\d{2}((0[1-9])|(1[0-2]))(([0-2][1-9])|10|20|30|31)\d{3}[0-9Xx]$/,
                    message: '请输入正确身份证号',
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
    computed: {
        vehicle_need_show: function () {
            var ret = [];
            if (this.search_key) {
                this.all_vehicle.forEach(element => {
                    if (PinyinMatch.match(element.main_vehicle_number, this.search_key) || PinyinMatch.match(element.behind_vehicle_number, this.search_key)) {
                        ret.push(element);
                    }
                });
            } else {
                ret = this.all_vehicle;
            }

            return ret;
        },
    },
    methods: {
        onRefresh: function () {
            this.init_all_vehicle();
        },
        proc_upload_vehicle: async function (_array) {
            var vue_this = this;
            for (var i = 0; i < _array.length; i++) {
                try {
                    await vue_this.$call_remote_process("vehicle_management", "add_vehicle", [vue_this.$cookies.get("zh_ssid"), _array[i]]);
                } catch (err) {
                    console.log(err);
                }
                vue_this.init_all_vehicle();
            }
        },
        format_black: function (value) {
            if (value.in_black_list) {
                return "是";
            } else {
                return "否";
            }
        },
        format_white: function (value) {
            if (value.in_white_list) {
                return "是";
            } else {
                return "否";
            }
        },
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
                vue_this.isLoading = false;
            });
        },
        clean_vehicle: function () {
            this.focus_vehicle = {
                main_vehicle_number: '',
                behind_vehicle_number: '',
                driver_name: '',
                driver_phone: '',
                driver_id: '',
                company_name: this.company_for_select[0].value,
                in_white_list: false,
                max_count: 35,
            };
        },
        edit_vehicle: function () {
            console.log(this.focus_vehicle);
            var vue_this = this;
            var func_name = "add_vehicle";
            if (!vue_this.current_opt_add) {
                func_name = "update_vehicle";
            }
            if (vue_this.focus_vehicle.company_name == '(自有)') {
                vue_this.focus_vehicle.company_name = "";
            }
            if (vue_this.focus_vehicle.company_name != "") {
                vue_this.focus_vehicle.in_white_list = false;
            }
            vue_this.focus_vehicle.main_vehicle_number = vue_this.focus_vehicle.main_vehicle_number.toUpperCase();
            vue_this.focus_vehicle.behind_vehicle_number = vue_this.focus_vehicle.behind_vehicle_number.toUpperCase();
            var func = function () {
                vue_this.$call_remote_process("vehicle_management", func_name, [vue_this.$cookies.get("zh_ssid"), vue_this.focus_vehicle]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_vehicle();
                        vue_this.show_edit_vehicle_diag = false;
                    }
                });
            };
            if (vue_this.$route.meta.mobile) {
                func();
            }
            vue_this.$refs.focus_vehicle.validate((valid) => {
                if (valid) {
                    func();
                }
            });
        },
    },
    beforeMount: function () {
        this.init_all_vehicle();
        var vue_this = this;
        vue_this.$call_remote_process("contract_management", 'get_all_contract', [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
            if (vue_this.$store.state.user_info.permission == 3) {
                vue_this.company_for_select = [];
            }
            resp.forEach(element => {
                vue_this.company_for_select.push({
                    value: element.name,
                    label: element.name,
                });
            });
            vue_this.focus_vehicle.company_name = vue_this.company_for_select[0].value;
        });
    }
}
</script>

<style scoped>
</style>
