<template>
<div class="contract_management_show">
    <div v-if="!$route.meta.mobile">
        <el-row type="flex" justify="space-between" align="middle">
            <el-col>
                <div class="block_title_show">所有合同</div>
            </el-col>
            <el-col>
                <div align="right" style="margin-right:10px;">
                    <el-input v-model="search_key" placeholder="输入公司名拼音首字母过滤" prefix-icon="el-icon-search"></el-input>
                </div>
            </el-col>
            <el-col>
                <div style="margin-right:10px; text-align:right;">
                    <el-button size="mini" type="primary" icon="el-icon-money" @click="charge_diag_show = true">充值</el-button>
                    <table-import-export @proc_table="proc_upload_contract" :sample_table="sample_table" export_name="合同导出表.xlsx" :export_table="all_contract" :item_name_map="col_map"></table-import-export>
                    <el-button size="mini" type="success" icon="el-icon-plus" @click="current_opt_add=true;show_add_contract_diag = true">新增</el-button>
                </div>
            </el-col>
        </el-row>
        <el-table :data="contract_need_show" style="width: 100%" stripe>
            <el-table-column type="index" label="编号" width="50px">
            </el-table-column>
            <el-table-column prop="name" label="公司名" width="200px">
            </el-table-column>
            <el-table-column prop="code" label="编码" width="150px">
            </el-table-column>
            <el-table-column label="关注物料" width="150px">
                <template slot-scope="scope">
                    <div v-for="(single_follow_stuff, index) in scope.row.follow_stuffs" :key="index">
                        {{single_follow_stuff}}
                    </div>
                </template>
            </el-table-column>
            <el-table-column label="类型" width="50px" :formatter="get_type">
            </el-table-column>
            <el-table-column prop="attachment" label="附件" width="80px">
                <template slot-scope="scope">
                    <el-link v-if="scope.row.attachment" type="primary" :href="$remote_file_url + scope.row.attachment">查看</el-link>
                    <span v-else>无附件</span>
                </template>
            </el-table-column>
            <el-table-column prop="admin_phone" label="管理员手机号" width="120px">
            </el-table-column>
            <el-table-column prop="balance" label="余额" width="120px">
                <template slot-scope="scope">
                    <span>{{scope.row.balance}}</span>
                    <el-tooltip class="item" effect="dark" content="历史余额" placement="top">
                        <el-button type="text" size="mini" @click="show_balance_history(scope.row)" class="el-icon-s-data"></el-button>
                    </el-tooltip>
                </template>
            </el-table-column>
            <el-table-column prop="credit" label="授信额度" width="80px">
            </el-table-column>
            <el-table-column prop="company_address" label="公司地址" width="200px">
            </el-table-column>
            <el-table-column fixed="right" label="操作" width="150px">
                <template slot-scope="scope">
                    <el-button type="warning" size="mini" @click="trigger_update_contract(scope.row)">修改</el-button>
                    <el-button type="danger" size="mini" @click="del_contract(scope.row)">删除</el-button>
                </template>
            </el-table-column>
        </el-table>
        <el-dialog @close="clean_contract" :title="(current_opt_add?'新增':'修改') + '合同'" :visible.sync="show_add_contract_diag" width="60%" @keyup.enter.native="add_contract" @open="gen_random_password">
            <el-form :model="new_contract" ref="add_contract_form" :rules="rules" label-width="120px">
                <el-form-item label="对方公司名称" prop="name">
                    <el-input v-model="new_contract.name" placeholder="请输入对方公司名称"></el-input>
                </el-form-item>
                <el-form-item label="关注物料" prop="follow_stuffs">
                    <el-row>
                        <el-col :span="12">
                            <item-for-select v-model="follow_stuff_need_add" search_key="stuff_name"></item-for-select>
                        </el-col>
                        <el-col :span="12">
                            <el-button icon="el-icon-plus" type="primary" circle @click="new_contract.follow_stuffs.push(follow_stuff_need_add);follow_stuff_need_add = ''"></el-button>
                        </el-col>
                    </el-row>
                    <el-tag v-for="(single_follow_stuff, index) in new_contract.follow_stuffs" :key="index" closable @close="new_contract.follow_stuffs.splice(index, 1)">
                        {{single_follow_stuff}}
                    </el-tag>
                </el-form-item>
                <el-form-item label="公司地址" prop="company_address">
                    <el-input v-model="new_contract.company_address" placeholder="请输入对方公司地址"></el-input>
                </el-form-item>
                <el-form-item label="管理员手机" prop="admin_phone">
                    <el-input v-model="new_contract.admin_phone" placeholder="请输入对方公司管理员手机号"></el-input>
                </el-form-item>
                <el-form-item label="密码" v-if="new_contract.admin_phone && old_admin_phone != new_contract.admin_phone">
                    <el-input v-model="new_contract.admin_password" disabled></el-input>
                    <el-button type="text" size="mini" @click="copy_random_password(new_contract.admin_password)">复制密码</el-button>
                    <span style="font-size:12px;">请复制密码，对话框关闭后将不再显示该密码</span>
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
                <el-form-item label="授信额度" prop="credit">
                    <el-input v-model="new_contract.credit" type="number" placeholder="请输入授信额度(可选)"></el-input>
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
        <el-drawer :title="balance_focus_company +'历史余额'" size="40%" :visible.sync="show_balance_history_diag" direction="rtl" @opened="open_balance_history_diag" @closed="close_balance_history_diag">
            <el-select v-model="history_type" placeholder="请选择">
                <el-option v-for="item in history_type_options" :key="item.value" :label="item.label" :value="item.value">
                </el-option>
            </el-select>
            <el-button type="warning" @click="advance_export_show = true">导出</el-button>
            <van-list ref="lazy_load" :offset="200" v-model="balance_history_loading" :finished="balance_history_load_end" finished-text="没有更多了" @load="get_balance_history">
                <el-table :data="balance_history_need_show" style="width: 100%" stripe>
                    <el-table-column label="时间" min-width="50px" prop="timestamp">
                    </el-table-column>
                    <el-table-column label="变化量" min-width="30px" prop="change_value">
                    </el-table-column>
                    <el-table-column label="修改后" min-width="30px" prop="new_value">
                    </el-table-column>
                    <el-table-column label="原因" min-width="60px" prop="reason">
                    </el-table-column>
                </el-table>
            </van-list>
        </el-drawer>
        <el-dialog title="导出" :visible.sync="advance_export_show" width="60%" @opened="history_need_export = [];begin_date='';end_date='';">
            <el-form ref="form" label-width="80px">
                <el-form-item label="日期范围">
                    <el-date-picker v-model="date_range" :picker-options="pickerOptions" type="datetimerange" range-separator="至" start-placeholder="开始日期" end-placeholder="结束日期" @change="advance_export_record">
                    </el-date-picker>
                </el-form-item>
                <table-import-export v-if="history_export_table.length > 0" no_need_import export_name="余额导出表.xlsx" :export_table="history_export_table" :item_name_map="het_col_map"></table-import-export>
            </el-form>
        </el-dialog>

    </div>
    <div v-else>
        <van-nav-bar title="合同管理" right-text="充值" @click-right="charge_diag_show = true">
        </van-nav-bar>
        <van-search v-model="search_key" placeholder="公司名过滤" />
        <van-cell v-for="(single_contract , index) in contract_need_show" is-link :key="index" :title="single_contract.name" :label="'余额:' + single_contract.balance" @click="show_qr = true;cur_qr_name = single_contract.name"></van-cell>
        <van-dialog v-model="show_qr" title="客户独立定价" :showConfirmButton="false" closeOnClickOverlay>
            <van-cell center v-for="(single_price, index) in focus_price_company" :key="index" :title="single_price.stuff_name" :value="single_price.price.toFixed(2)">
                <template #right-icon>
                    <van-button size="mini" type="warning" @click="focus_customer_stuff_price.id = single_price.id;focus_customer_stuff_price.stuff_name = single_price.stuff_name;focus_customer_stuff_price.price = single_price.price;focus_add = false;show_edit_price = true;focus_customer_stuff_price.customer_name = cur_qr_name">
                        修改
                    </van-button>
                    <van-button size="mini" type="danger" @click="delete_stuff_price(single_price.id)">删除</van-button>
                </template>
            </van-cell>
            <div style="margin:12px;">
                <van-button type="primary" block size="small" @click="show_edit_price = true;focus_add = true;focus_customer_stuff_price.customer_name = cur_qr_name">新增</van-button>
            </div>
        </van-dialog>

        <van-dialog @close="focus_customer_stuff_price.id = 0" v-model="show_edit_price" :title="focus_add?'新增':'修改' + '独立定价'" :showConfirmButton="false" closeOnClickOverlay>
            <van-form @submit="edit_stuff_price">
                <van-field :rules="[{ required: true, message: '请指定物料' }]" readonly :disabled="!focus_add" clickable name="picker" :value="focus_customer_stuff_price.stuff_name" label="物料" placeholder="点击选择物料" @click="show_stuff_picker= true" />
                <van-popup get-container="body" v-model="show_stuff_picker" position="bottom">
                    <van-picker show-toolbar :columns="all_stuffs" @confirm="confirm_pick_stuff" @cancel="show_stuff_picker = false" />
                </van-popup>
                <van-field v-model="focus_customer_stuff_price.price" label="单价" type="number" placeholder="请输入单价" :rules="[{ required: true, message: '请输入单价' }]" />
                <div style="margin: 16px;">
                    <van-button round block type="info" native-type="submit">保存</van-button>
                </div>
            </van-form>
        </van-dialog>
    </div>

    <van-dialog v-if="$route.meta.mobile" v-model="charge_diag_show" title="充值单" :showConfirmButton="false" closeOnClickOverlay>
        <van-form @submit="change_balance">
            <item-for-select label="客户名称" v-model="charge_company" :rules="[{required:true, message:'请指定公司'}]" search_key="company_name"></item-for-select>
            <item-for-select label="用途物料" v-model="charge_stuff" :rules="[{required:true, message:'请指定物料名'}]" search_key="stuff_name"></item-for-select>
            <van-field required v-model="charge_value" name="充值金额" label="充值金额" type="number" placeholder="" :rules="[{required:true, message:'请指定金额'}]" />
            <van-field required v-model="charge_reason" name="充值原因" label="充值原因" type="text" placeholder="" :rules="[{required:true, message:'请指定原因'}]" />
            <div style="margin: 16px;">
                <van-button round block type="info" native-type="submit">提交</van-button>
            </div>
        </van-form>
    </van-dialog>
    <el-dialog v-else title="充值" :visible.sync="charge_diag_show" width="60%" @keyup.enter.native="change_balance">
        <el-form label-width="120px" ref="charge_form">
            <el-form-item label="客户名称" prop="charge_company">
                <item-for-select v-model="charge_company" :rules="[{required:true, message:'请指定公司'}]" search_key="company_name"></item-for-select>
            </el-form-item>
            <el-form-item label="用途物料" prop="charge_stuff">
                <item-for-select v-model="charge_stuff" :rules="[{required:true, message:'请指定物料名'}]" search_key="stuff_name"></item-for-select>
            </el-form-item>
            <el-form-item label="充值金额" prop="charge_value">
                <el-input v-model="charge_value" placeholder="请输入充值金额" :rules="[{required:true, message:'请指定金额'}]"></el-input>
            </el-form-item>
            <el-form-item label="充值原因" prop="charge_reason">
                <el-input v-model="charge_reason" placeholder="请输入充值原因" :rules="[{required:true, message:'请指定原因'}]"></el-input>
            </el-form-item>

            <el-form-item>
                <el-button type="primary" @click="change_balance">提交</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</div>
</template>

<script>
import Vue from 'vue'
import TableImportExport from '../components/TableImportExport.vue'
import VueClipboard from 'vue-clipboard2'
import Vant from 'vant';
import 'vant/lib/index.css';
import ItemForSelect from "../components/ItemForSelect.vue"
import PinyinMatch from "pinyin-match"
Vue.use(Vant);
Vue.use(VueClipboard)
export default {
    name: 'ContractManagement',
    components: {
        "table-import-export": TableImportExport,
        "item-for-select": ItemForSelect,
    },
    data: function () {
        return {
            show_edit_price: false,
            all_stuffs: [],
            show_stuff_picker: false,
            focus_add: false,
            focus_customer_stuff_price: {
                id: 0,
                stuff_name: '',
                customer_name: '',
                price: 0
            },
            pickerOptions: {
                shortcuts: [{
                    text: '最近一周',
                    onClick(picker) {
                        const end = new Date();
                        const start = new Date();
                        start.setTime(start.getTime() - 3600 * 1000 * 24 * 7);
                        picker.$emit('pick', [start, end]);
                    }
                }, {
                    text: '最近一个月',
                    onClick(picker) {
                        const end = new Date();
                        const start = new Date();
                        start.setTime(start.getTime() - 3600 * 1000 * 24 * 30);
                        picker.$emit('pick', [start, end]);
                    }
                }, {
                    text: '最近三个月',
                    onClick(picker) {
                        const end = new Date();
                        const start = new Date();
                        start.setTime(start.getTime() - 3600 * 1000 * 24 * 90);
                        picker.$emit('pick', [start, end]);
                    }
                }]
            },
            history_export_table: [],
            het_col_map: {
                company_name: {
                    text: "公司名称"
                },
                timestamp: {
                    text: "时间"
                },
                change_value: {
                    text: '变化量'
                },
                reason: {
                    text: '原因'
                },
                type: {
                    text: '类型'
                }
            },
            begin_date: '',
            end_date: '',
            history_need_export: [],
            advance_export_show: false,
            history_type: 0,
            history_type_options: [{
                    value: 0,
                    label: '全部'
                },
                {
                    value: 1,
                    label: '充值'
                },
                {
                    value: 2,
                    label: '消费'
                },
            ],
            all_customer_stuff_price: [],
            charge_company: '',
            charge_stuff: '',
            charge_value: '',
            charge_reason: '',
            charge_diag_show: false,
            domain_name: '',
            show_qr: false,
            cur_qr_name: '',
            search_key: '',
            active: 0,
            follow_stuff_need_add: '',
            balance_history: [],
            balance_history_load_end: false,
            balance_history_loading: false,
            show_balance_history_diag: false,
            sample_table: [{
                name: '客户公司名',
                company_address: "内蒙古通辽",
                code: '合同编号1',
                is_sale: true,
                balance: 0,
            }, {
                name: '供应商公司名',
                company_address: "新疆伊犁",
                code: '合同编号2',
                is_sale: false,
                balance: 0,
            }],
            old_admin_phone: '',
            current_opt_add: true,
            all_contract: [],
            show_add_contract_diag: false,
            new_contract: {
                name: '',
                code: '',
                attachment: '',
                is_sale: true,
                admin_phone: '',
                admin_password: '',
                credit: 0,
                follow_stuffs: [],
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
            col_map: {
                name: {
                    text: '公司名',
                },
                company_address: {
                    text: '公司地址',
                },
                code: {
                    text: '编码',
                },
                is_sale: {
                    text: '类型',
                    formatter: function (_orig) {
                        if (_orig) {
                            return "销售";
                        } else {
                            return "采购";
                        }
                    },
                    parser(_value) {
                        if (_value == "销售") {
                            return true;
                        } else {
                            return false;
                        }
                    },
                },
                balance: {
                    text: '余额',
                    formatter: function (_orig) {
                        return _orig.toFixed(2);
                    },
                    parser(_value) {
                        return parseFloat(_value);
                    },
                },
            },
            balance_focus_company: '',
        };
    },
    computed: {
        focus_price_company: function () {
            var ret = [];

            this.all_customer_stuff_price.forEach(element => {
                if (element.customer_name == this.cur_qr_name) {
                    ret.push(element);
                }
            });

            return ret;
        },
        date_range: {
            get: function () {
                var ret = [];
                ret.push(this.begin_date);
                ret.push(this.end_date);
                return ret;
            },
            set: function (_new_value) {
                this.begin_date = _new_value[0];
                this.end_date = _new_value[1];
            }
        },
        balance_history_need_show: function () {
            var ret = [];
            this.balance_history.forEach(element => {
                if (this.$store.state.focus_stuff.length > 0 && !element.reason.includes(this.$store.state.focus_stuff)) {
                    return;
                }
                if (this.history_type == 0) {
                    ret.push(element);
                } else if (this.history_type == 1 && !element.reason.includes('（系统自动）')) {
                    ret.push(element);
                } else if (this.history_type == 2 && element.reason.includes('（系统自动）')) {
                    ret.push(element);
                }
            });
            return ret;
        },
        contract_need_show: function () {
            var ret = [];

            this.all_contract.forEach(element => {
                if (this.search_key) {
                    if (PinyinMatch.match(element.name, this.search_key)) {
                        ret.push(element);
                    }
                } else {
                    ret.push(element);
                }
            });
            if (this.$store.state.focus_stuff) {
                var tmp_ret = ret;
                ret = [];
                tmp_ret.forEach(element => {
                    if (element.follow_stuffs.find(item => {
                            return item == this.$store.state.focus_stuff;
                        })) {
                        var tmp_item = element;
                        tmp_item.follow_stuffs = [this.$store.state.focus_stuff];
                        ret.push(element);
                    }
                });

            }

            return ret;
        },
    },
    methods: {
        delete_stuff_price: function (_id) {
            var vue_this = this;
            this.$dialog.confirm({
                title: '删除确认',
                message: "确定要删除吗?"
            }).then(function () {
                vue_this.$call_remote_process("contract_management", "del_single_contract_price", [vue_this.$cookies.get("zh_ssid"), _id]).then(function (resp) {
                    if (resp) {
                        vue_this.init_stuff_company();
                    }
                });
            });
        },
        init_all_stuff: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", 'get_all_stuff', [vue_this.$cookies.get('zh_ssid')]).then(function (resp) {
                vue_this.all_stuffs = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_stuffs, index, element.name);
                });
            });
        },
        edit_stuff_price: function () {
            var vue_this = this;
            vue_this.$call_remote_process("contract_management", "add_single_contract_price", [vue_this.$cookies.get("zh_ssid"), vue_this.focus_customer_stuff_price]).then(function (resp) {
                if (resp) {
                    vue_this.show_edit_price = false;
                    vue_this.init_stuff_company();
                }
            });
        },
        confirm_pick_stuff: function (_stuff_name) {
            this.focus_customer_stuff_price.stuff_name = _stuff_name;
            this.show_stuff_picker = false;
        },
        init_stuff_company: function () {
            var vue_this = this;
            vue_this.$call_remote_process("contract_management", "get_all_single_contract_price", []).then(function (resp) {
                vue_this.all_customer_stuff_price = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_customer_stuff_price, index, element);
                });
            });
        },
        advance_export_record: function () {
            var vue_this = this;
            vue_this.$call_remote_process("contract_management", "export_history", [vue_this.$cookies.get("zh_ssid"), vue_this.$make_time_string(vue_this.begin_date, '-'), vue_this.$make_time_string(vue_this.end_date, '-'), vue_this.balance_focus_company]).then(function (resp) {
                vue_this.history_export_table = [];
                resp.forEach(element => {
                    var tmp_item = {
                        ...element
                    };
                    tmp_item.company_name = vue_this.balance_focus_company;
                    if (tmp_item.reason.includes('（系统自动）')) {
                        tmp_item.type = "消费";
                    } else {
                        tmp_item.type = "充值";
                    }
                    vue_this.history_export_table.push(tmp_item);
                });
            });
        },
        open_balance_history_diag: function () {
            this.$refs.lazy_load.check();
        },
        get_balance_history: function () {
            var vue_this = this;
            if (vue_this.balance_focus_company) {
                vue_this.$call_remote_process("contract_management", "get_history", [vue_this.$cookies.get("zh_ssid"), vue_this.balance_focus_company, vue_this.balance_history.length]).then(function (resp) {
                    vue_this.balance_history_loading = false;
                    resp.forEach((element) => {
                        vue_this.balance_history.push(element)
                    });
                    if (resp.length < 10) {
                        vue_this.balance_history_load_end = true;
                    }
                });

            } else {
                vue_this.balance_history_load_end = true;
            }
        },
        show_balance_history: function (_contract) {
            this.balance_focus_company = _contract.name;
            this.show_balance_history_diag = true;
        },
        close_balance_history_diag: function () {
            this.balance_focus_company = '';
            this.balance_history = [];
            this.balance_history_load_end = false;
            this.balance_history_loading = false;
        },
        change_balance: function () {
            var vue_this = this;
            var func = () => {
                var real_reason = "关联物料：" + vue_this.charge_stuff + ", 原因:" + vue_this.charge_reason;
                vue_this.$call_remote_process("contract_management", "change_balance", [vue_this.$cookies.get("zh_ssid"), vue_this.charge_company, parseFloat(vue_this.charge_value), real_reason]).then(function (resp) {
                    if (resp) {
                        vue_this.init_all_contract();
                    }
                }).finally(function () {
                    vue_this.charge_value = 0;
                    vue_this.charge_company = '';
                    vue_this.charge_stuff = '';
                    vue_this.charge_reason = '';
                    vue_this.charge_diag_show = false;
                });
            };
            if (vue_this.charge_value && vue_this.charge_company && vue_this.charge_stuff && vue_this.charge_reason) {
                func()
            } else {
                vue_this.$alert('信息不完整');
            }

        },
        proc_upload_contract: async function (_array) {
            var vue_this = this;
            for (var i = 0; i < _array.length; i++) {
                try {
                    await vue_this.$call_remote_process("contract_management", "add_contract", [vue_this.$cookies.get("zh_ssid"), _array[i]]);
                } catch (err) {
                    console.log(err);
                }
                vue_this.init_all_contract();
            }
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
            this.new_contract.admin_password = this.make_random_passwod();
        },
        copy_random_password: function (_password) {
            this.$copyText(_password);
            this.$message('密码已复制');
        },
        clean_contract: function () {
            this.new_contract = {
                name: '',
                code: '',
                attachment: '',
                is_sale: true,
                admin_phone: '',
                admin_password: '',
                credit: 0,
                follow_stuffs: [],
            };
            this.old_admin_phone = "";
        },
        trigger_update_contract: function (_contract) {
            var vue_this = this;
            vue_this.new_contract = {
                ..._contract
            };
            vue_this.old_admin_phone = _contract.admin_phone;
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
                    var shajs = require('sha.js')
                    vue_this.new_contract.admin_password = shajs('sha1').update(vue_this.new_contract.admin_password).digest('hex');
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
        var vue_this = this;
        vue_this.$call_remote_process("system_management", "get_domain_name", []).then(function (resp) {
            vue_this.domain_name = resp;
        });
        this.init_stuff_company();
        this.init_all_stuff();
    },
}
</script>

<style scoped>
</style>
