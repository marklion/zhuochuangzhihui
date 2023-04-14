<template>
<div class="stuff_management_sh">

    <el-tabs v-model="activeName">
        <el-tab-pane label="物料维护" name="stuff_config">
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
                <el-table-column type="index" label="编号" width="50px">
                </el-table-column>
                <el-table-column prop="name" label="物料名" width="120px">
                </el-table-column>
                <el-table-column prop="code" label="编码" width="120px">
                </el-table-column>
                <el-table-column prop="unit" label="计量单位" width="100px">
                </el-table-column>
                <el-table-column prop="inventory" label="库存" width="60px">
                </el-table-column>
                <el-table-column prop="price" label="单价" width="120px">
                    <template slot-scope="scope">
                        <span>{{scope.row.price}}</span>
                        <el-tooltip class="item" effect="dark" content="修改价格" placement="top">
                            <el-button type="text" size="mini" @click="change_price(scope.row)" class="el-icon-edit"></el-button>
                        </el-tooltip>
                        <el-tooltip class="item" effect="dark" content="历史价格" placement="top">
                            <el-button type="text" size="mini" @click="show_price_history(scope.row)" class="el-icon-s-data"></el-button>
                        </el-tooltip>
                    </template>
                </el-table-column>
                <el-table-column label="需要矿(厂)发净重" width="80px">
                    <template slot-scope="scope">
                        {{scope.row.need_enter_weight?'是':'否'}}
                    </template>
                </el-table-column>
                <el-table-column prop="expect_weight" label="预计装车净重" width="120px">
                </el-table-column>
                <el-table-column label="手动称重" width="80px">
                    <template slot-scope="scope">
                        {{scope.row.need_manual_scale?'是':'否'}}
                    </template>
                </el-table-column>

                <el-table-column label="用于白名单" width="80px">
                    <template slot-scope="scope">
                        {{scope.row.use_for_white_list?'是':'否'}}
                    </template>
                </el-table-column>
                <el-table-column label="自动确认装卸货" width="80px">
                    <template slot-scope="scope">
                        {{scope.row.auto_confirm_deliver?'是':'否'}}
                    </template>
                </el-table-column>
                <el-table-column label="需要一次称重磅单" width="80px">
                    <template slot-scope="scope">
                        {{scope.row.need_p_ticket?'是':'否'}}
                    </template>
                </el-table-column>
                <el-table-column prop="min_limit" label="最小装车量" width="120px">
                </el-table-column>
                <el-table-column prop="max_limit" label="最大装车量" width="120px">
                </el-table-column>
                <el-table-column fixed="right" label="操作" width="150px">
                    <template slot-scope="scope">
                        <el-button type="warning" size="mini" @click="trigger_update_stuff(scope.row)">修改</el-button>
                        <el-button type="danger" size="mini" @click="del_stuff(scope.row)">删除</el-button>
                    </template>
                </el-table-column>
            </el-table>
        </el-tab-pane>

        <el-tab-pane label="物料源维护" name="stuff_source">
            <el-row type="flex" justify="space-between" align="middle">
                <el-col>
                    <div class="block_title_show">所有物料源</div>
                </el-col>
                <el-col>
                    <div style="margin-right:10px; text-align:right">
                        <el-button size="mini" type="success" icon="el-icon-plus" @click="show_edit_stuff_source_dest_diag = true;focus_stuff_source_dest.is_source = true;">新增</el-button>
                    </div>
                </el-col>
            </el-row>

            <el-table :data="all_stuff_source" style="width: 100%" stripe>
                <el-table-column type="index" label="编号" width="50px">
                </el-table-column>
                <el-table-column prop="name" label="物料源" width="120px">
                </el-table-column>
                <el-table-column fixed="right" label="操作" width="150px">
                    <template slot-scope="scope">
                        <el-button type="danger" size="mini" @click="del_stuff_source_dest(scope.row)">删除</el-button>
                    </template>
                </el-table-column>
            </el-table>
        </el-tab-pane>
        <el-tab-pane label="物料目的地维护" name="stuff_dest">
            <el-row type="flex" justify="space-between" align="middle">
                <el-col>
                    <div class="block_title_show">所有物料目的地</div>
                </el-col>
                <el-col>
                    <div style="margin-right:10px; text-align:right">
                        <el-button size="mini" type="success" icon="el-icon-plus" @click="show_edit_stuff_source_dest_diag = true;focus_stuff_source_dest.is_source = false">新增</el-button>
                    </div>
                </el-col>
            </el-row>
            <el-table :data="all_stuff_dest" style="width: 100%" stripe>
                <el-table-column type="index" label="编号" width="50px">
                </el-table-column>
                <el-table-column prop="name" label="物料目的地" width="120px">
                </el-table-column>
                <el-table-column prop="code" label="编码" width="120px">
                </el-table-column>
                <el-table-column fixed="right" label="操作" width="150px">
                    <template slot-scope="scope">
                        <el-button type="danger" size="mini" @click="del_stuff_source_dest(scope.row)">删除</el-button>
                    </template>
                </el-table-column>
            </el-table>
        </el-tab-pane>
        <el-tab-pane label="客户独立定价" name="single_contract_stuff_price">
            <el-row type="flex" justify="space-between" align="middle">
                <el-col>
                    <div class="block_title_show">所有独立定价</div>
                </el-col>
                <el-col>
                    <div style="margin-right:10px; text-align:right">
                        <el-button size="mini" type="success" icon="el-icon-plus" @click="open_customer_price_diag({}, true)">新增</el-button>
                    </div>
                </el-col>
            </el-row>
            <el-table :data="all_customer_stuff_price" style="width: 100%" stripe>
                <el-table-column type="index" label="编号" width="50px">
                </el-table-column>
                <el-table-column prop="customer_name" label="客户名称" width="320px">
                </el-table-column>
                <el-table-column prop="stuff_name" label="物料名称" width="120px">
                </el-table-column>
                <el-table-column prop="price" label="单价" width="120px">
                </el-table-column>
                <el-table-column label="操作" width="320px">
                    <template slot-scope="scope">
                        <el-button type="text" size="mini" @click="show_single_stuff_price_history(scope.row)" class="el-icon-s-data">修改历史</el-button>
                        <el-button type="warning" size="mini" @click="open_customer_price_diag(scope.row, false)">修改</el-button>
                        <el-button type="danger" size="mini" @click="del_customer_stuff_price(scope.row)">删除</el-button>
                    </template>
                </el-table-column>
            </el-table>
        </el-tab-pane>
        <el-tab-pane label="货仓配置" name="stuff_inv_info">
            <el-button size="small" type="primary" @click="add_stuff_inv_info">新增</el-button>
            <el-descriptions v-for="(single_sii, index) in all_sii" :key="index" :title="single_sii.name" :column="4" size="small" border>
                <template slot="extra">
                    <el-button type="success" size="small" @click="open_edit_sii(single_sii)">编辑</el-button>
                    <el-button type="danger" size="small" @click="del_stuff_inv_info(single_sii)">删除</el-button>
                </template>
                <el-descriptions-item v-for="(single_sie, sie_index) in single_sii.sie" :key="sie_index" :label="single_sie.stuff_name">{{single_sie.inventory}}</el-descriptions-item>
            </el-descriptions>
        </el-tab-pane>
    </el-tabs>

    <el-dialog :title="(focus_customer_stuff_price.id == 0?'新增':'修改') + '独立定价'" :visible.sync="customer_stuff_diag" width="60%" @keyup.enter.native="opt_customer_stuff_price">
        <el-form :model="focus_customer_stuff_price" ref="focus_customer_stuff_price_form" label-width="150px">
            <el-form-item label="公司名称" prop="customer_name">
                <item-for-select v-model="focus_customer_stuff_price.customer_name" search_key="company_name"></item-for-select>
            </el-form-item>
            <el-form-item label="物料名称" prop="stuff_name">
                <item-for-select v-model="focus_customer_stuff_price.stuff_name" search_key="stuff_name"></item-for-select>
            </el-form-item>
            <el-form-item label="单价" prop="price">
                <el-input v-model="focus_customer_stuff_price.price" placeholder="请输入物料单价"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="opt_customer_stuff_price">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
    <el-dialog @close="clean_stuff" :title="(current_opt_add?'新增':'修改') + '物料'" :visible.sync="show_edit_stuff_diag" width="60%" @keyup.enter.native="edit_stuff">
        <el-form :model="focus_stuff" ref="edit_stuff_form" :rules="rules" label-width="150px">
            <el-form-item label="物料名称" prop="name">
                <el-input v-model="focus_stuff.name" placeholder="请输入物料名称"></el-input>
            </el-form-item>
            <el-form-item label="物料编码" prop="code">
                <el-input v-model="focus_stuff.code" placeholder="请输入物料编码"></el-input>
            </el-form-item>
            <el-form-item label="计量单位" prop="unit">
                <el-autocomplete v-model="focus_stuff.unit" :fetch-suggestions="fetch_unit" placeholder="请输入物料单位"></el-autocomplete>
            </el-form-item>
            <el-form-item label="库存" prop="inventory">
                <el-input-number v-model="focus_stuff.inventory" :min="0.001" label="请输入库存"></el-input-number>
            </el-form-item>
            <el-form-item label="预计装车净重" prop="expect_weight">
                <el-input-number v-model="focus_stuff.expect_weight" :min="0.001" label="请输入预计装车净重"></el-input-number>
            </el-form-item>
            <el-form-item label="需要矿(厂)发净重" prop="inventory">
                <el-switch v-model="focus_stuff.need_enter_weight" active-color="#13ce66" inactive-color="#ff4949">
                </el-switch>
            </el-form-item>
            <el-form-item label="用于白名单" prop="use_for_white_list">
                <el-switch v-model="focus_stuff.use_for_white_list" active-color="#13ce66" inactive-color="#ff4949">
                </el-switch>
            </el-form-item>
            <el-form-item label="手动称重" prop="inventory">
                <el-switch v-model="focus_stuff.need_manual_scale" active-color="#13ce66" inactive-color="#ff4949">
                </el-switch>
            </el-form-item>
            <el-form-item label="最小装车量" prop="min_limit">
                <el-input-number v-model="focus_stuff.min_limit" :min="0.001" label="请输入最小装车量"></el-input-number>
            </el-form-item>
            <el-form-item label="最大装车量" prop="max_limit">
                <el-input-number v-model="focus_stuff.max_limit" :min="0.001" label="请输入最大装车量"></el-input-number>
            </el-form-item>
            <el-form-item label="自动装卸货确认" prop="auto_confirm_deliver">
                <el-switch v-model="focus_stuff.auto_confirm_deliver" active-color="#13ce66" inactive-color="#ff4949">
                </el-switch>
            </el-form-item>
            <el-form-item label="需要一次称重磅单" prop="need_p_ticket">
                <el-switch v-model="focus_stuff.need_p_ticket" active-color="#13ce66" inactive-color="#ff4949">
                </el-switch>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="edit_stuff">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
    <el-dialog @close="clean_stuff_source_dest" :title="'新增物料' + (focus_stuff_source_dest.is_source?'源':'目的地')" :visible.sync="show_edit_stuff_source_dest_diag" width="60%" @keyup.enter.native="edit_stuff_source_dest">
        <el-form :model="focus_stuff_source_dest" ref="edit_stuff_source_dest_form" label-width="150px">
            <el-form-item label="名称" prop="name" :rules="[{required:true, message:'请输入名称'}]">
                <el-input v-model="focus_stuff_source_dest.name" placeholder="请输入名称"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="edit_stuff_source_dest">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
    <el-drawer :title="price_focus_stuff+'历史单价'" size="40%" :visible.sync="show_price_history_diag" direction="rtl" @opened="open_price_history_diag" @closed="close_price_history_diag">
        <van-list ref="lazy_load" :offset="200" v-model="price_history_loading" :finished="price_history_load_end" finished-text="没有更多了" @load="get_price_history">
            <el-table :data="price_history" style="width: 100%" stripe>
                <el-table-column label="时间" min-width="50px" prop="timestamp">
                </el-table-column>
                <el-table-column label="变化量" min-width="30px" prop="change_value">
                </el-table-column>
                <el-table-column label="修改后" min-width="30px" prop="new_value">
                </el-table-column>
            </el-table>
        </van-list>
    </el-drawer>

    <el-dialog @close="close_edit_sii" title="编辑货仓" :visible.sync="edit_sii_diag" width="60%" @keyup.enter.native="update_stuff_inv_info">
        <el-form :model="focus_sii" ref="edit_sii" label-width="150px">
            <el-form-item label="货仓名称" prop="name">
                <el-input v-model="focus_sii.name" placeholder="请输入货仓名称"></el-input>
            </el-form-item>
            <el-form-item v-for="(single_sie, index) in  focus_sii.sie" :key="index" :label="single_sie.stuff_name">
                <el-input v-model="single_sie.inventory" type="number" placeholder="请输入库存"></el-input>
                <el-button @click.prevent="del_stuff_from_sie(single_sie.stuff_name)">删除</el-button>
            </el-form-item>

            <el-form-item>
                <el-button type="primary" @click="add_sub_stuff">新增物料</el-button>
                <el-button type="success" @click="update_stuff_inv_info">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</div>
</template>

<script>
import TableImportExport from '../components/TableImportExport.vue'
import PinyinMatch from "pinyin-match"
import Vue from 'vue'
import Vant from 'vant';
import 'vant/lib/index.css';
import ItemForSelect from "../components/ItemForSelect.vue"
Vue.use(Vant);
export default {
    name: 'StuffManagement',
    components: {
        "table-import-export": TableImportExport,
        "item-for-select": ItemForSelect,
    },
    data: function () {
        return {
            focus_sii: {},
            edit_sii_diag: false,
            all_sii: [],
            customer_stuff_diag: false,
            focus_customer_stuff_price: {
                id: 0,
                stuff_name: '',
                customer_name: '',
                price: 0
            },
            all_customer_stuff_price: [],
            all_stuff_dest: [],
            all_stuff_source: [],
            show_edit_stuff_source_dest_diag: false,
            activeName: 'stuff_config',
            focus_stuff_source_dest: {
                id: 0,
                name: "",
                is_source: false,
            },
            price_focus_stuff: '',
            price_focus_single_id: 0,
            price_history: [],
            price_history_load_end: false,
            price_history_loading: false,
            show_price_history_diag: false,
            current_opt_add: true,
            show_edit_stuff_diag: false,
            focus_stuff: {
                id: 0,
                name: '',
                unit: '',
                inventory: 0,
                expect_weight: 30,
                need_enter_weight: false,
                need_manual_scale: false,
                max_limit: 49.5,
                min_limit: 48.5,
                code: "",
                auto_confirm_deliver:false,
                need_p_ticket:false,
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
                need_enter_weight: {
                    text: '需要矿(厂)发净重',
                    formatter: function (_orig) {
                        if (_orig) {
                            return "是";
                        } else {
                            return "否";
                        }
                    },
                    parser(_value) {
                        if (_value == "是") {
                            return true;
                        } else {
                            return false;
                        }
                    },
                },
                price: {
                    text: '单价',
                    formatter: function (_orig) {
                        return _orig.toFixed(2);
                    },
                    parser(_value) {
                        return parseFloat(_value);
                    },
                },
                expect_weight: {
                    text: '预计装车净重',
                    formatter: function (_orig) {
                        return _orig.toFixed(2);
                    },
                    parser(_value) {
                        return parseFloat(_value);
                    },
                },
            },
            sample_table: [{
                name: '中水',
                unit: '吨',
                inventory: '104',
                need_enter_weight: false,
                price: 600,
                expect_weight: 30,
            }],
        };
    },
    methods: {
        open_customer_price_diag: function (_bound_price, is_add) {
            if (is_add) {
                this.focus_customer_stuff_price = {
                    id: 0,
                    stuff_name: '',
                    customer_name: '',
                    price: 0,
                };
            }

            this.focus_customer_stuff_price = _bound_price;
            this.customer_stuff_diag = true;
        },
        init_customer_stuff_price: function () {
            var vue_this = this;
            vue_this.$call_remote_process("contract_management", "get_all_single_contract_price", []).then(function (resp) {
                vue_this.all_customer_stuff_price = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_customer_stuff_price, index, element);
                });
                if (vue_this.$store.state.focus_stuff) {
                    var tmp_ret = [];
                    vue_this.all_customer_stuff_price.forEach(item => {
                        if (item.stuff_name == vue_this.$store.state.focus_stuff) {
                            tmp_ret.push(item);
                        }
                    });
                    vue_this.all_customer_stuff_price = tmp_ret;
                }
            });
        },
        opt_customer_stuff_price: function () {
            var vue_this = this;
            vue_this.focus_customer_stuff_price.price = parseFloat(vue_this.focus_customer_stuff_price.price);
            vue_this.$call_remote_process("contract_management", "add_single_contract_price", [vue_this.$cookies.get("zh_ssid"), vue_this.focus_customer_stuff_price]).then(function (resp) {
                if (resp) {
                    vue_this.customer_stuff_diag = false;
                    vue_this.init_customer_stuff_price();
                }
            });
        },
        del_customer_stuff_price: function (_bound_price) {
            var vue_this = this;

            this.$confirm('确定删除吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("contract_management", "del_single_contract_price", [vue_this.$cookies.get("zh_ssid"), _bound_price.id]).then(function (resp) {
                    if (resp) {
                        vue_this.init_customer_stuff_price();
                    }
                });

            });
        },
        open_price_history_diag: function () {
            this.$refs.lazy_load.check();
        },
        get_price_history: async function () {
            var vue_this = this;
            var resp = [];
            if (vue_this.price_focus_stuff) {
                if (vue_this.price_focus_single_id != 0) {
                    resp = await vue_this.$call_remote_process("contract_management", "get_single_price_history", [vue_this.$cookies.get("zh_ssid"), vue_this.price_focus_single_id, vue_this.price_history.length]);
                } else {
                    resp = await vue_this.$call_remote_process("stuff_management", "get_history", [vue_this.$cookies.get("zh_ssid"), vue_this.price_focus_stuff, vue_this.price_history.length]);
                }
                resp.forEach((element) => {
                    vue_this.price_history.push(element)
                });
                if (resp.length < 10) {
                    vue_this.price_history_load_end = true;
                }
            } else {
                vue_this.price_history_load_end = true;
            }
        },
        show_single_stuff_price_history: function (_single_price) {
            this.price_focus_stuff = _single_price.stuff_name;
            this.price_focus_single_id = _single_price.id;
            this.show_price_history_diag = true;
        },
        show_price_history: function (_stuff) {
            this.price_focus_stuff = _stuff.name;
            this.show_price_history_diag = true;
        },
        close_price_history_diag: function () {
            this.price_focus_stuff = '';
            this.price_focus_single_id = 0;
            this.price_history = [];
            this.price_history_load_end = false;
            this.price_history_loading = false;
        },
        change_price: function (_stuff) {
            var vue_this = this;
            this.$prompt('请输入调整后的单价', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                inputType: 'number',
                inputPattern: /^[\s\S]*.*[^\s][\s\S]*$/,
                inputErrorMessage: '请输入新单价'
            }).then(function (_new_price) {
                vue_this.$call_remote_process("stuff_management", "change_price", [vue_this.$cookies.get("zh_ssid"), _stuff.name, _new_price.value]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_stuff();
                    }
                });
            });

        },
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
        del_stuff_source_dest: function (_stuff) {
            var vue_this = this;
            this.$confirm('确定删除吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("stuff_management", "del_source_dest", [vue_this.$cookies.get("zh_ssid"), _stuff.id]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_stuff_source_dest();
                    }
                });
            });

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
        edit_stuff_source_dest: function () {
            var vue_this = this;
            var func_name = "add_source_dest";
            vue_this.$refs.edit_stuff_source_dest_form.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("stuff_management", func_name, [vue_this.$cookies.get("zh_ssid"), vue_this.focus_stuff_source_dest.name, vue_this.focus_stuff_source_dest.is_source]).then(function (resp) {
                        if (resp) {
                            vue_this.show_edit_stuff_source_dest_diag = false;
                            vue_this.init_all_stuff_source_dest();
                        }
                    });
                }
            });
        },
        init_all_stuff: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", 'get_all_stuff', [vue_this.$cookies.get('zh_ssid')]).then(function (resp) {
                vue_this.all_stuff = [];
                if (vue_this.$store.state.focus_stuff) {
                    var found_item = resp.find(item => {
                        return item.name == vue_this.$store.state.focus_stuff;
                    });
                    vue_this.$set(vue_this.all_stuff, 0, found_item);
                } else {

                    resp.forEach((element, index) => {
                        vue_this.$set(vue_this.all_stuff, index, element);
                    });
                }
            });
        },
        init_all_stuff_source_dest: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", 'get_all_source_dest', [true]).then(function (resp) {
                vue_this.all_stuff_source = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_stuff_source, index, element);
                });
            });
            vue_this.$call_remote_process("stuff_management", 'get_all_source_dest', [false]).then(function (resp) {
                vue_this.all_stuff_dest = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_stuff_dest, index, element);
                });
            });
        },
        clean_stuff: function () {
            this.focus_stuff = {
                id: 0,
                name: '',
                unit: '',
                inventory: 0,
                expect_weight: 30,
                need_enter_weight: false,
                need_manual_scale: false,
                max_limit: 49.5,
                min_limit: 48.5,
                code: "",
            };
        },
        clean_stuff_source_dest: function () {
            this.focus_stuff_source_dest = {
                id: 0,
                name: "",
                is_source: false,
            };
        },
        add_sub_stuff: function () {
            this.$prompt('请输入物料名', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
            }).then(({
                value
            }) => {
                this.focus_sii.sie.push({
                    stuff_name: value,
                    inventory: 0
                });
            });
        },
        init_stuff_inv_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", "get_stuff_inv_info", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.all_sii = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_sii, index, element);
                });
            });
        },
        add_stuff_inv_info: function () {
            var vue_this = this;
            this.$prompt('请输入货仓名', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
            }).then(({
                value
            }) => {
                vue_this.$call_remote_process("stuff_management", "add_stuff_inv_info", [vue_this.$cookies.get("zh_ssid"), value]).then(function (resp) {
                    if (resp) {
                        vue_this.init_stuff_inv_info();
                    }
                });
            });
        },
        del_stuff_inv_info: function (_sii) {
            this.$confirm('确认删除' + _sii.name + '吗？', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                var vue_this = this;
                vue_this.$call_remote_process("stuff_management", "del_stuff_inv_info", [vue_this.$cookies.get("zh_ssid"), _sii.id]).then(function () {
                    vue_this.init_stuff_inv_info();
                });

            });
        },
        update_stuff_inv_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", "update_stuff_inv_info", [vue_this.$cookies.get("zh_ssid"), vue_this.focus_sii]).then(function (resp) {
                if (resp) {
                    vue_this.init_stuff_inv_info();
                    vue_this.edit_sii_diag = false;
                }
            });
        },
        open_edit_sii: function (_sii) {
            this.focus_sii = {
                ..._sii
            };
            this.edit_sii_diag = true;
        },
        close_edit_sii: function () {
            this.focus_sii = {};
            this.init_stuff_inv_info();
        },
        del_stuff_from_sie: function (_stuff_name) {
            for (var i = 0; i < this.focus_sii.sie.length; i++) {
                if (this.focus_sii.sie[i].stuff_name == _stuff_name) {
                    this.focus_sii.sie.splice(i, 1);
                    break;
                }
            }
        },
    },
    beforeMount: function () {
        this.init_all_stuff();
        this.init_all_stuff_source_dest();
        this.init_customer_stuff_price();
        this.init_stuff_inv_info();
    },

}
</script>

<style scoped>

</style>
