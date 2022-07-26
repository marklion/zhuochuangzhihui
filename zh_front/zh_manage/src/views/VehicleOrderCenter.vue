<template>
<div class="vehicle_order_center_show">
    <div v-if="!$route.meta.mobile">
        <el-row type="flex" justify="space-between" align="middle">
            <el-col>
                <div class="block_title_show">派车中心</div>
            </el-col>
            <el-col>
                <div align="right" style="margin-right:10px;">
                    <el-button size="mini" type="info" icon="el-icon-notebook-2" v-if="order_selected.length > 0" @click="export_xlsx">导出所选{{order_selected.length}}项</el-button>
                    <el-button size="mini" type="primary" icon="el-icon-check" v-if="order_selected.length > 0" @click="confirm_multi">批量确认{{order_selected.length}}项</el-button>
                    <el-button size="mini" type="danger" icon="el-icon-s-release" v-if="order_selected.length > 0" @click="cancle_multi">批量取消{{order_selected.length}}项</el-button>
                    <el-button size="mini" type="success" icon="el-icon-plus" @click="current_opt_add=true;show_edit_order_diag = true">新增</el-button>
                    <el-button size="mini" type="info" icon="el-icon-notebook-2" @click="advance_export_show = true">高级导出</el-button>
                </div>
            </el-col>
        </el-row>
        <el-dialog title="高级导出" :visible.sync="advance_export_show" width="60%">
            <el-form ref="form" label-width="80px">
                <el-form-item label="称重日期范围">
                    <el-date-picker v-model="date_range" type="datetimerange" range-separator="至" start-placeholder="开始日期" end-placeholder="结束日期">
                    </el-date-picker>
                </el-form-item>
                <el-form-item label="公司名">
                    <item-for-select :disabled="$store.state.user_info.permission==3" v-model="focus_comapny" search_key="company_name"></item-for-select>
                </el-form-item>
                <el-form-item label="物料名">
                    <item-for-select :disabled="$store.state.user_info.permission==3" v-model="focus_stuff" search_key="stuff_name"></item-for-select>
                </el-form-item>
                <el-form-item>
                    <el-button type="primary" @click="advance_export_record">导出</el-button>
                </el-form-item>
            </el-form>
        </el-dialog>
        <el-row type="flex" justify="space-between">
            <el-col :span="12">
                <el-tabs v-model="activeName" @tab-click="refresh_order">
                    <el-tab-pane label="所有派车单" name="all">
                    </el-tab-pane>
                    <el-tab-pane label="未确认" name="need_confirm">
                        <template slot="label">
                            <span>未确认</span>
                            <el-badge :value="cur_statistic.unconfirm" v-if="cur_statistic.unconfirm>0" size="mini"></el-badge>
                        </template>
                    </el-tab-pane>
                    <el-tab-pane name="has_not_came">
                        <template slot="label">
                            <span>未入场</span>
                            <el-badge :value="cur_statistic.not_in_yet" v-if="cur_statistic.not_in_yet>0" size="mini"></el-badge>
                        </template>
                    </el-tab-pane>
                    <el-tab-pane name="insite">
                        <template slot="label">
                            <span>已入场</span>
                            <el-badge :value="cur_statistic.already_in" v-if="cur_statistic.already_in>0" size="mini"></el-badge>
                        </template>
                    </el-tab-pane>
                    <el-tab-pane label="已完成" name="end">
                    </el-tab-pane>
                </el-tabs>
            </el-col>
            <el-col :span="6">
                <div align="right" style="margin-right:10px;">
                    <el-date-picker @change="choose_date" v-model="enter_date_filter" align="right" type="date" placeholder="过滤进厂日期" :picker-options="picker_option" value-format="yyyy-MM-dd">
                    </el-date-picker>
                </div>
            </el-col>
            <el-col :span="6">
                <div align="right" style="margin-right:10px;">
                    <el-input @input="recheck_list" v-model="search_condition" placeholder="输入公司名拼音首字母/车牌号过滤" prefix-icon="el-icon-search"></el-input>
                </div>
            </el-col>
        </el-row>
        <el-popover ref="pop1" v-model="visible1" width="200">
            <company-balance :company_name="curObj1"></company-balance>
        </el-popover>
        <el-popover ref="pop2" v-model="visible2" width="200">
            <stuff-price :stuff_name="curObj2"></stuff-price>
        </el-popover>
        <van-list ref="lazy_load" :offset="2000" v-model="is_loading" :finished="lazy_finish" finished-text="没有更多了" @load="get_order">
            <el-table :data="order_need_show" @selection-change="proc_order_select" style="width: 100%" stripe ref="order_table">
                <el-table-column type="selection" width="30px">
                </el-table-column>
                <el-table-column label="单号" width="150px">
                    <template slot-scope="scope">
                        <router-link tag="el-link" :to="{name:'VehicleDetail', params:{order_no:scope.row.order_number}}">
                            <el-link type="primary">{{scope.row.order_number}}</el-link>
                        </router-link>
                    </template>
                </el-table-column>
                <el-table-column label="公司" width="250px">
                    <template slot-scope="scope">
                        <div slot="reference" @mouseenter="e=> show_pop_info1(e, scope.row.company_name)" @mouseleave="visible1 = false">{{scope.row.company_name}}</div>
                    </template>
                </el-table-column>
                <el-table-column label="运输货物" width="130px">
                    <template slot-scope="scope">
                        <div slot="reference" @mouseenter="e=> show_pop_info2(e, scope.row.stuff_name)" @mouseleave="visible2 = false">{{scope.row.stuff_name}}</div>
                    </template>
                </el-table-column>
                <el-table-column label="状态" width="200px">
                    <template slot-scope="scope">
                        <div>
                            {{scope.row.status_details[scope.row.status_details.length - 1].name}}
                        </div>
                        <div v-if="scope.row.status == 0" style="color:red;">
                            {{scope.row.balance_warn}}
                        </div>
                        <div v-if="scope.row.status == 3 && deliver_cost_time(calc_status_date(scope.row, 2)?calc_status_date(scope.row,2):calc_status_date(scope.row,3)) > 30" style="color:red">
                            装卸货消耗{{deliver_cost_time(calc_status_date(scope.row, 2)?calc_status_date(scope.row,2):calc_status_date(scope.row,3))}}分钟
                        </div>
                        <div v-if="scope.row.end_time && scope.row.status != 100">
                            <el-tag type="success">{{scope.row.end_time}} 连续派车</el-tag>
                        </div>
                    </template>
                </el-table-column>
                <el-table-column label="车牌号" width="100px" prop="main_vehicle_number">
                </el-table-column>
                <el-table-column label="净重" width="100px">
                    <template slot-scope="scope">
                        <div v-if="scope.row.status >= 4">
                            {{Math.abs(scope.row.m_weight - scope.row.p_weight).toFixed(2)}}
                        </div>
                        <div v-else>
                            未完成
                        </div>
                    </template>
                </el-table-column>
                <el-table-column width="40px" type="expand">
                    <template slot-scope="scope">
                        <div>
                            <el-descriptions size="mini" :column="8" border>
                                <el-descriptions-item label="一次称重">{{scope.row.p_weight}}</el-descriptions-item>
                                <el-descriptions-item label="二次称重">{{scope.row.m_weight}}</el-descriptions-item>
                                <el-descriptions-item label="司机">{{scope.row.driver_name}}</el-descriptions-item>
                                <el-descriptions-item label="电话">{{scope.row.driver_phone}}</el-descriptions-item>
                                <el-descriptions-item label="身份证">{{scope.row.driver_id}}</el-descriptions-item>
                            </el-descriptions>
                        </div>
                    </template>
                </el-table-column>
                <el-table-column label="进厂时间" width="180px">
                    <template slot-scope="scope">
                        {{calc_status_date(scope.row, 2)?calc_status_date(scope.row,2):calc_status_date(scope.row,3)}}
                    </template>
                </el-table-column>
                <el-table-column label="出厂时间" width="180px">
                    <template slot-scope="scope">
                        {{calc_status_date(scope.row, 100)}}
                    </template>
                </el-table-column>
                <el-table-column label="附件" width="100px">
                    <template slot-scope="scope">
                        <el-image v-if="scope.row.attachment" style="width: 100%; height: 40px;" :src="$remote_file_url + scope.row.attachment" :preview-src-list="[$remote_file_url + scope.row.attachment]">
                        </el-image>
                        <div v-else>
                            无附件
                        </div>
                        <div v-if="scope.row.status <= 1">
                            <el-button v-if="scope.row.attachment" icon="el-icon-delete" type="warning" size="mini" @click="delete_attachment(scope.row)">删除</el-button>
                            <el-upload v-else accept="image/*" :action="$remote_url + '/upload/'" :show-file-list="false" :limit="1" :on-success="upload_attachment(scope.row)">
                                <el-button size="mini" type="primary" icon="el-icon-paperclip">上传</el-button>
                            </el-upload>
                        </div>
                    </template>
                </el-table-column>
                <el-table-column fixed="right" label="操作" width="120px">
                    <template slot-scope="scope">

                        <span v-if="scope.row.status < 2">
                            <el-tooltip class="item" effect="dark" content="取消派车" placement="top">
                                <el-button type="danger" size="small" icon="el-icon-s-release" @click="cancel_order([scope.row])" circle></el-button>
                            </el-tooltip>
                        </span>
                        <span v-if="$store.state.user_info.permission <= 1 && scope.row.status == 0">
                            <el-tooltip class="item" effect="dark" content="确认派车" placement="top">
                                <el-button type="success" size="small" icon="el-icon-check" circle @click="confirm_order([scope.row])"></el-button>
                            </el-tooltip>
                        </span>
                        <el-popover placement="right" width="400" trigger="click" v-if="scope.row.status == 100">
                            <div>
                                <ticket :order_number="scope.row.order_number">
                                </ticket>
                            </div>
                            <el-button size="small" slot="reference">查看磅单</el-button>
                        </el-popover>
                    </template>
                </el-table-column>
            </el-table>
        </van-list>
        <el-dialog @open="fill_company_name" @close="clean_order" :title="(current_opt_add?'新增':'修改') + '派车单'" :visible.sync="show_edit_order_diag" width="60%" @keyup.enter.native="edit_order">
            <el-row type="flex" justify="space-between" align="middle">
                <el-col :span="10">
                    <el-row type="flex" justify="space-between" align="middle">
                        <el-col :span="12" v-if="focus_order.company_name">
                            <company-balance :company_name="focus_order.company_name"></company-balance>
                        </el-col>
                        <el-col :span="12" v-if="focus_order.stuff_name">
                            <stuff-price :stuff_name="focus_order.stuff_name"></stuff-price>
                        </el-col>
                    </el-row>
                    <el-form :model="focus_order" ref="edit_order_form" :rules="rules" label-width="120px">
                        <el-form-item label="派车公司" prop="company_name">
                            <item-for-select :disabled="$store.state.user_info.permission==3" v-model="focus_order.company_name" search_key="company_name"></item-for-select>
                        </el-form-item>
                        <el-form-item label="运输货物" prop="stuff_name">
                            <item-for-select v-model="focus_order.stuff_name" search_key="stuff_name"></item-for-select>
                        </el-form-item>
                        <el-form-item label="今天连续派车">
                            <el-switch v-model="form_continue_switch"></el-switch>
                        </el-form-item>
                        <el-form-item>
                            <el-button type="primary" @click="edit_order">确认</el-button>
                        </el-form-item>
                    </el-form>
                </el-col>
                <el-col :span="12">
                    <div class="all_vehicle_select_show">
                        <el-button type="success" size="mini" @click="show_vehicle_select = true">增加</el-button>
                        <el-empty v-if="vehicle_selected.length <= 0" description="未添加车辆"></el-empty>
                        <div v-else>
                            <div class="single_vehicle_show" v-for="(single_vehicle, index) in vehicle_selected" :key="index">
                                <el-descriptions size="mini" :column="2" border :title="single_vehicle.main_vehicle_number + '-' + single_vehicle.behind_vehicle_number">
                                    <template slot="extra">
                                        <el-button type="danger" size="mini" @click="remove_single_vehicle(index)">移除</el-button>
                                    </template>
                                    <el-descriptions-item label="司机">
                                        <span>
                                            {{single_vehicle.driver_name}}
                                        </span>
                                        <el-button style="float:right;padding:0;" type="text" @click="tmp_change('司机', single_vehicle)">修改</el-button>
                                    </el-descriptions-item>
                                    <el-descriptions-item label="电话">
                                        <span>
                                            {{single_vehicle.driver_phone}}
                                        </span>
                                        <el-button style="float:right;padding:0;" type="text" @click="tmp_change('电话', single_vehicle)">修改</el-button>
                                    </el-descriptions-item>
                                    <el-descriptions-item label="身份证">
                                        <span>
                                            {{single_vehicle.driver_id}}
                                        </span>
                                        <el-button style="float:right;padding:0;" type="text" @click="tmp_change('身份证', single_vehicle)">修改</el-button>
                                    </el-descriptions-item>
                                    <el-descriptions-item label="最大净重">
                                        <span>
                                            {{single_vehicle.max_count}}
                                        </span>
                                        <el-button style="float:right;padding:0;" type="text" @click="tmp_change('最大净重', single_vehicle)">修改</el-button>
                                    </el-descriptions-item>
                                </el-descriptions>
                            </div>
                        </div>
                    </div>
                </el-col>
            </el-row>
        </el-dialog>

    </div>
    <div v-else>
        <van-pull-refresh v-model="isLoading" @refresh="onRefresh">
            <van-nav-bar title="派车中心" right-text="新增" left-text="退出" @click-right="create_mobile_vehicle_diag = true" @click-left="user_logoff" />
            <van-tabs v-model="activeName" @change="refresh_order">
                <van-tab title="所有" name="all"></van-tab>
                <van-tab title="未确认" name="need_confirm" :badge="cur_statistic.unconfirm>0?cur_statistic.unconfirm:''"></van-tab>
                <van-tab title="未入场" name="has_not_came" :badge="cur_statistic.not_in_yet>0?cur_statistic.not_in_yet:''"></van-tab>
                <van-tab title="已入场" name="insite" :badge="cur_statistic.already_in>0?cur_statistic.already_in:''"></van-tab>
                <van-tab title="结束" name="end"></van-tab>

            </van-tabs>
            <el-input @input="recheck_list" v-model="search_condition" placeholder="输入公司名拼音首字母/车牌号过滤" prefix-icon="el-icon-search"></el-input>
            <van-list ref="lazy_load" :offset="2000" v-model="is_loading" :finished="lazy_finish" finished-text="没有更多了" @load="get_order">
                <van-cell v-for="(single_vehicle, index) in order_need_show" :key="index" center :to="{name:'MobileVehicleDetail', params:{order_no:single_vehicle.order_number}}" :value="single_vehicle.stuff_name" :label="single_vehicle.company_name">
                    <template #right-icon>
                        <div style="margin-left:10px;" v-if="single_vehicle.status != 100">
                            <van-button v-if="$store.state.user_info.permission <= 1 && single_vehicle.status == 0" type="primary" size="mini" @click="confirm_order([single_vehicle])">确认</van-button>
                            <van-button v-if="single_vehicle.status <= 1" size="mini" type="danger" @click="cancel_order([single_vehicle])">取消</van-button>
                        </div>
                    </template>
                    <template #title>
                        <span>{{single_vehicle.main_vehicle_number}}-{{single_vehicle.driver_name}}</span>
                        <van-tag size="mini" type="danger">
                            {{single_vehicle.status_details[single_vehicle.status_details.length - 1].name}}
                        </van-tag>
                        <van-tag size="mini" type="primary" v-if="single_vehicle.end_time && single_vehicle.status != 100">
                            {{single_vehicle.end_time}} 连续派车
                        </van-tag>
                        <div v-if="single_vehicle.status == 0" style="color:red;">
                            {{single_vehicle.balance_warn}}
                        </div>
                        <div v-if="single_vehicle.status == 3 && deliver_cost_time(calc_status_date(single_vehicle, 2)?calc_status_date(single_vehicle,2):calc_status_date(single_vehicle,3)) > 30" style="color:red">
                            装卸货消耗{{deliver_cost_time(calc_status_date(single_vehicle, 2)?calc_status_date(single_vehicle,2):calc_status_date(single_vehicle,3))}}分钟
                        </div>
                    </template>
                </van-cell>
            </van-list>

            <van-dialog v-model="create_mobile_vehicle_diag" title="新建派车" :showConfirmButton="false" closeOnClickOverlay>
                <van-form @submit="mobile_create_order">
                    <item-for-select label="派车公司" v-model="focus_order.company_name" :rules="[{required:true, message:'请指定公司'}]" search_key="company_name"></item-for-select>
                    <item-for-select label="运输货物" v-model="focus_order.stuff_name" :rules="[{required:true, message:'请指定货物'}]" search_key="stuff_name"></item-for-select>
                    <van-field name="switch" label="连续派车">
                        <template #input>
                            <van-switch v-model="form_continue_switch" size="20" />
                        </template>
                    </van-field>
                    <van-field readonly clickable name="picker" label="选择车辆" placeholder="点击选择车辆" @click="show_vehicle_select = true">
                    </van-field>
                    <van-tag v-for="(single_vehicle,index) in vehicle_selected" :key="index" closeable size="mini" type="primary" @close="remove_single_vehicle(index)">
                        {{single_vehicle.main_vehicle_number}}
                    </van-tag>
                    <div style="margin: 16px;">
                        <van-button round block type="info" native-type="submit">提交</van-button>
                    </div>
                </van-form>
            </van-dialog>
        </van-pull-refresh>
    </div>
    <el-drawer @closed="clean_select" :visible.sync="show_vehicle_select" direction="rtl" size="70%">
        <div slot="title">
            <div>请选择车辆</div>
            <el-button size="small" type="primary" @click="push_ready_to_select">确认</el-button>
        </div>
        <el-table :data="vehicle_for_select" style="width: 100%" ref="vehicle_select_table" stripe @selection-change="proc_select">
            <el-table-column type="selection" width="55" :selectable="verify_selectable">
            </el-table-column>
            <el-table-column sortable property="main_vehicle_number" label="主车牌" width="120"></el-table-column>
            <el-table-column sortable property="behind_vehicle_number" label="挂车牌" width="120"></el-table-column>
            <el-table-column sortable property="driver_name" label="司机" width="100"></el-table-column>
            <el-table-column sortable property="driver_phone" label="司机电话" width="120"></el-table-column>
            <el-table-column sortable property="driver_id" label="司机身份证" width="150"></el-table-column>
            <el-table-column sortable property="company_name" label="所属公司" width="150"></el-table-column>
            <el-table-column sortable property="group_name" label="分组名" width="100"></el-table-column>
            <el-table-column sortable property="max_count" label="最大净重" width="100"></el-table-column>
        </el-table>
        <div align="right" style="margin-right:10px;">
            <el-button type="primary" @click="push_ready_to_select">确认</el-button>
        </div>
    </el-drawer>
</div>
</template>

<script>
import Vue from 'vue';
import ItemForSelect from "../components/ItemForSelect.vue"
import CompanyBalance from "../components/CompanyBalance.vue"
import StuffPrice from "../components/StuffPrice.vue"
import infiniteScroll from 'vue-infinite-scroll'
import Ticket from '../components/Ticket.vue'

import PinyinMatch from "pinyin-match"
import XLSX from 'xlsx';
import VueClipboard from 'vue-clipboard2'

import Vant from 'vant';
import 'vant/lib/index.css';
Vue.use(Vant);
Vue.use(VueClipboard)
Vue.use(infiniteScroll)
export default {
    name: 'VehicleOrderCenter',
    components: {
        "item-for-select": ItemForSelect,
        "company-balance": CompanyBalance,
        "stuff-price": StuffPrice,
        "ticket": Ticket
    },
    events: {
        ['need_refresh']() {
            console.log("tick");
        }
    },
    computed: {
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
        need_fetch: function () {
            var ret = false;
            if (!this.lazy_finish && !this.is_loading) {
                ret = true;
            }
            return ret;
        },
        order_need_show: function () {
            var ret = [];

            this.all_order.forEach(element => {
                switch (this.activeName) {
                    case 'all':
                        ret.push(element);
                        break;
                    case 'need_confirm':
                        if (element.status == 0) {
                            ret.push(element);
                        }
                        break;
                    case 'has_not_came':
                        if (element.status == 1) {
                            ret.push(element);
                        }
                        break;
                    case 'insite':
                        if (element.status > 1 && element.status < 100) {
                            ret.push(element);
                        }
                        break;
                    case 'end':
                        if (element.status == 100) {
                            ret.push(element);
                        }
                        break;
                    default:
                        break;
                }
            });
            var tmp_ret = ret;
            ret = [];
            tmp_ret.forEach(element => {
                if (this.search_condition.length <= 0) {
                    ret.push(element);
                } else if (PinyinMatch.match(element.company_name, this.search_condition) || PinyinMatch.match(element.main_vehicle_number, this.search_condition)) {
                    ret.push(element);
                }
            });

            if (this.$store.state.focus_stuff) {
                tmp_ret = ret;
                ret = [];
                tmp_ret.forEach(element => {
                    if (element.stuff_name == this.$store.state.focus_stuff) {
                        ret.push(element);
                    }
                });
            }

            return ret;
        },
    },
    data: function () {
        return {
            focus_comapny: '',
            focus_stuff: '',
            begin_date: '',
            end_date: '',
            advance_export_show: false,
            form_continue_switch: false,
            create_mobile_vehicle_diag: false,
            isLoading: false,
            deliver_cost_time: function (_start_time) {
                var start_time = new Date(_start_time);
                var ms = new Date().getTime() - start_time.getTime();
                return Math.floor(ms / 1000 / 60);
            },
            pop_info_component: undefined,
            picker_option: {
                disabledDate(time) {
                    return time.getTime() > Date.now();
                },
                shortcuts: [{
                    text: '今天',
                    onClick(picker) {
                        picker.$emit('pick', new Date());
                    }
                }, {
                    text: '昨天',
                    onClick(picker) {
                        const date = new Date();
                        date.setTime(date.getTime() - 3600 * 1000 * 24);
                        picker.$emit('pick', date);
                    }
                }, {
                    text: '一周前',
                    onClick(picker) {
                        const date = new Date();
                        date.setTime(date.getTime() - 3600 * 1000 * 24 * 7);
                        picker.$emit('pick', date);
                    }
                }]
            },
            enter_date_filter: '',
            calc_status_date: function (_row, _status) {
                var ret = "";

                _row.status_details.forEach(element => {
                    if (element.step == _status) {
                        ret = element.timestamp;
                    }
                });

                return ret;
            },
            cur_statistic: {
                unconfirm: 0,
                not_in_yet: 0,
                already_in: 0,
            },
            order_selected: [],
            search_condition: '',
            activeName: 'all',
            lazy_finish: false,
            is_loading: false,
            ready_vehicle: [],
            show_vehicle_select: false,
            rules: {
                company_name: [{
                    required: true,
                    message: "请选择派车公司",
                    trigger: ['blur', 'change']
                }],
                stuff_name: [{
                    required: true,
                    message: "请选择运送的货物",
                    trigger: ['blur', 'change']
                }],
            },
            vehicle_selected: [],
            vehicle_for_select: [],
            current_opt_add: false,
            show_edit_order_diag: false,
            all_order: [],
            focus_order: {
                company_name: '',
                company_address: '',
                use_for: '',
                main_vehicle_number: '',
                behind_vehicle_number: '',
                driver_id: '',
                driver_phone: '',
                driver_name: '',
                stuff_name: '',
            },
            domain_name: '',
            curObj1: '',
            curObj2: '',
            visible1: false,
            visible2: false,
        };
    },
    methods: {
        advance_export_record: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "export_order_by_condition", [vue_this.$cookies.get("zh_ssid"), vue_this.$make_time_string(vue_this.begin_date, '-'), vue_this.$make_time_string(vue_this.end_date, '-'), vue_this.focus_comapny, vue_this.focus_stuff]).then(function (resp) {
                var tmp = [];
                resp.forEach(element => {
                    tmp.push(element.basic_info);
                });
                vue_this.export_xlsx(tmp);
            });
        },
        user_logoff: function () {
            var vue_this = this;
            vue_this.$call_remote_process("user_management", 'user_logoff', [vue_this.$cookies.get('zh_ssid')]).then(function () {
                vue_this.$cookies.set('zh_ssid', "");
                vue_this.$router.go(0);
            });
        },
        cancle_multi: function () {
            this.cancel_order(this.order_selected);
        },
        confirm_multi: function () {
            this.confirm_order(this.order_selected);
        },
        diag_func: function (_message, _title) {
            if (this.$route.meta.mobile) {
                return this.$dialog.confirm({
                    title: _title,
                    message: _message
                });
            } else {
                return this.$confirm(_message, _title, {
                    confirmButtonText: '确认',
                    cancelButtonText: '取消',
                    type: 'warning'
                });
            }
        },
        mobile_create_order: function () {
            var vue_this = this;
            var req_body = {};
            var func_name = "";
            req_body = [];
            vue_this.vehicle_selected.forEach(element => {
                var single_req = element;
                single_req.company_name = vue_this.focus_order.company_name;
                single_req.stuff_name = vue_this.focus_order.stuff_name;
                single_req.company_address = vue_this.focus_order.company_address;
                single_req.use_for = vue_this.focus_order.use_for;
                if (vue_this.form_continue_switch) {
                    single_req.end_time = vue_this.$make_time_string(new Date(), '-').substr(0, 10);
                }
                req_body.push(single_req);
            });
            func_name = "create_vehicle_order";
            vue_this.$call_remote_process("vehicle_order_center", func_name, [vue_this.$cookies.get("zh_ssid"), req_body]).then(function (resp) {
                if (resp) {
                    vue_this.refresh_order();
                    vue_this.create_mobile_vehicle_diag = false;
                }
            });
        },
        onRefresh: function () {
            this.refresh_order()
        },
        tmp_change: function (_type, _vehicle) {
            this.$prompt('请输入' + _type, '修改', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                inputPattern: /^[\s\S]*.*[^\s][\s\S]*$/,
                inputErrorMessage: _type + '格式不正确'
            }).then(({
                value
            }) => {
                switch (_type) {
                    case '司机':
                        _vehicle.driver_name = value;
                        break;
                    case '电话':
                        _vehicle.driver_phone = value;
                        break;
                    case '身份证':
                        _vehicle.driver_id = value;
                        break;
                    case '最大净重':
                        _vehicle.max_count = value;
                        break;

                    default:
                        break;
                }
            });
        },
        show_pop_info2: function (e, obj) {
            this.curObj2 = obj
            //关键代码
            //先隐藏并销毁之前显示的
            this.visible2 = false
            var pop2 = this.$refs.pop2
            pop2.doDestroy(true)
            this.$nextTick(() => {
                //显示新的
                pop2.referenceElm = pop2.$refs.reference = e.target
                this.visible2 = true
            })
        },
        show_pop_info1: function (e, obj) {
            this.curObj1 = obj
            //关键代码
            //先隐藏并销毁之前显示的
            this.visible1 = false
            var pop1 = this.$refs.pop1
            pop1.doDestroy(true)
            this.$nextTick(() => {
                //显示新的
                pop1.referenceElm = pop1.$refs.reference = e.target
                this.visible1 = true
            })
        },
        choose_date: function () {
            this.recheck_list();
        },
        fill_company_name: function () {
            if (this.$store.state.user_info.permission == 3) {
                this.focus_order.company_name = this.$store.state.user_info.name;
            }
        },
        copy_check_in_link: function (_order) {
            this.$copyText('http://' + this.domain_name + '/#/mobile/check_in/' + _order.order_number);
            this.$message('链接已复制，建议发送给司机');
        },
        upload_attachment: function (_order) {
            var vue_this = this;
            return function (resp, file) {
                var real_path = resp.match(/^\/tmp\/.*/gm)[0];
                var file_name_split = file.name.split('.');
                var changed_order = {
                    ..._order
                };
                changed_order.attachment = real_path + '.' + file_name_split[file_name_split.length - 1];
                vue_this.$call_remote_process("vehicle_order_center", "update_vehicle_order", [vue_this.$cookies.get("zh_ssid"), changed_order]).then(function (resp) {
                    if (resp) {
                        vue_this.$call_remote_process("vehicle_order_center", "get_order_detail", [vue_this.$cookies.get("zh_ssid"), changed_order.order_number]).then(function (fresh_order) {
                            _order.attachment = fresh_order.basic_info.attachment;
                        });
                    }
                });
            };
        },
        delete_attachment: function (_order) {
            var vue_this = this;
            var changed_order = {
                ..._order
            };
            changed_order.attachment = "";
            vue_this.$call_remote_process("vehicle_order_center", "update_vehicle_order", [vue_this.$cookies.get("zh_ssid"), changed_order]).then(function (resp) {
                if (resp) {
                    vue_this.$call_remote_process("vehicle_order_center", "get_order_detail", [vue_this.$cookies.get("zh_ssid"), changed_order.order_number]).then(function (fresh_order) {
                        _order.attachment = fresh_order.basic_info.attachment;
                    });
                }
            });
        },
        proc_order_select: function (_value) {
            this.order_selected = _value;
        },
        cancel_order: function (orders) {
            var vue_this = this;
            vue_this.diag_func('确定要取消吗', '提示', {
                confirmButtonText: '确认',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("vehicle_order_center", "cancel_vehicle_order", [vue_this.$cookies.get("zh_ssid"), orders]).then(function (resp) {
                    if (resp) {
                        vue_this.refresh_order();
                    }
                });
            })
        },
        confirm_order: function (orders) {
            var vue_this = this;
            var need_confirm_balance = false;
            orders.forEach(element => {
                if (element.balance_warn) {
                    need_confirm_balance = true;
                }
            });
            var real_func = function () {
                vue_this.$call_remote_process("vehicle_order_center", "confirm_vehicle_order", [vue_this.$cookies.get("zh_ssid"), orders]).then(function (resp) {
                    if (resp) {
                        vue_this.refresh_order();
                    }
                });
            };
            if (need_confirm_balance) {
                vue_this.diag_func('车辆所属的公司余额不足，确定要确认派车吗', '提示', {
                    confirmButtonText: '强制确认',
                    cancelButtonText: '取消',
                    type: 'warning'
                }).then(() => {
                    real_func();
                })
            } else {
                real_func();
            }

        },
        remove_single_vehicle: function (index) {
            this.vehicle_selected.splice(index, 1);
        },
        verify_selectable: function (row) {
            var ret = true;
            if (this.vehicle_selected.find((itr => {
                    return itr.id == row.id
                }))) {
                ret = false;
            }

            return ret;
        },
        clean_select: function () {
            this.ready_vehicle = [];
            this.$refs.vehicle_select_table.clearSelection();
        },
        push_ready_to_select: function () {
            var vue_this = this;
            this.ready_vehicle.forEach(element => {
                vue_this.vehicle_selected.push(element);
            });
            vue_this.show_vehicle_select = false;
        },
        proc_select: function (_value) {
            this.ready_vehicle = _value;
        },
        clean_order: function () {
            this.form_continue_switch = false;
            this.focus_order = {
                company_name: '',
                company_address: '',
                use_for: '',
                main_vehicle_number: '',
                behind_vehicle_number: '',
                driver_id: '',
                driver_phone: '',
                driver_name: '',
                stuff_name: '',
            }
            this.vehicle_selected = [];
        },
        init_statistics: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_order_statistics", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.cur_statistic.unconfirm = resp.created;
                vue_this.cur_statistic.not_in_yet = resp.confirmed;
                vue_this.cur_statistic.already_in = resp.entered + resp.first_weight + resp.second_weight;
            });
        },
        recheck_list: function () {
            this.lazy_finish = false;
            this.$refs.lazy_load.check();
        },
        refresh_order: function () {
            var vue_this = this;
            console.log(this.activeName);
            vue_this.all_order = [];
            vue_this.lazy_finish = false;
            this.$refs.lazy_load.check();
            this.init_statistics();
        },
        edit_order: function () {
            var vue_this = this;
            var req_body = {};
            var func_name = "";
            if (vue_this.current_opt_add) {
                req_body = [];
                vue_this.vehicle_selected.forEach(element => {
                    var single_req = element;
                    single_req.company_name = vue_this.focus_order.company_name;
                    single_req.stuff_name = vue_this.focus_order.stuff_name;
                    single_req.company_address = vue_this.focus_order.company_address;
                    single_req.use_for = vue_this.focus_order.use_for;
                    if (vue_this.form_continue_switch) {
                        single_req.end_time = vue_this.$make_time_string(new Date(), '-').substr(0, 10);
                    }
                    req_body.push(single_req);
                });
                func_name = "create_vehicle_order";
            } else {
                req_body = vue_this.focus_order;
            }
            vue_this.$refs.edit_order_form.validate((valid) => {
                if (valid) {
                    var real_add_req_func = () => {
                        vue_this.$call_remote_process("vehicle_order_center", func_name, [vue_this.$cookies.get("zh_ssid"), req_body]).then(function (resp) {
                            if (resp) {
                                vue_this.refresh_order();
                                vue_this.show_edit_order_diag = false;
                            }
                        });
                    };
                    vue_this.$call_remote_process("vehicle_order_center", "check_price_balance", [vue_this.$cookies.get("zh_ssid"), req_body]).then(function (balance_msg) {
                        if (balance_msg) {
                            vue_this.diag_func(balance_msg, '提示', {
                                confirmButtonText: '仍然派车',
                                cancelButtonText: '取消',
                                type: 'warning'
                            }).then(() => {
                                real_add_req_func();
                            });
                        } else {
                            real_add_req_func();
                        }
                    });

                }
            });
        },
        get_order: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_order_by_anchor", [vue_this.$cookies.get("zh_ssid"), vue_this.all_order.length, vue_this.activeName, vue_this.enter_date_filter]).then(function (resp) {
                resp.forEach(element => {
                    vue_this.all_order.push(element);
                });
                if (resp.length <= 0) {
                    vue_this.lazy_finish = true;
                }
            }).finally(function () {
                vue_this.is_loading = false;
                vue_this.isLoading = false;
            });

        },
        // 导出xlsx
        exportExcel: function (headers, data, fileName = '记录表.xlsx') {
            const _headers = headers
                .map((item, i) => Object.assign({}, {
                    key: item.key,
                    title: item.title,
                    position: String.fromCharCode(65 + i) + 1
                }))
                .reduce((prev, next) => Object.assign({}, prev, {
                    [next.position]: {
                        key: next.key,
                        v: next.title
                    }
                }), {});

            const _data = data
                .map((item, i) => headers.map((key, j) => Object.assign({}, {
                    content: item[key.key],
                    position: String.fromCharCode(65 + j) + (i + 2)
                })))
                // 对刚才的结果进行降维处理（二维数组变成一维数组）
                .reduce((prev, next) => prev.concat(next))
                // 转换成 worksheet 需要的结构
                .reduce((prev, next) => Object.assign({}, prev, {
                    [next.position]: {
                        v: next.content
                    }
                }), {});

            // 合并 headers 和 data
            const output = Object.assign({}, _headers, _data);
            // 获取所有单元格的位置
            const outputPos = Object.keys(output);
            // 计算出范围 ,["A1",..., "H2"]
            const ref = `${outputPos[0]}:${outputPos[outputPos.length - 1]}`;

            // 构建 workbook 对象
            const wb = {
                SheetNames: ['mySheet'],
                Sheets: {
                    mySheet: Object.assign({},
                        output, {
                            '!ref': ref,
                            '!cols': [{
                                wpx: 80
                            }, {
                                wpx: 180
                            }, {
                                wpx: 70
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 60
                            }, {
                                wpx: 120
                            }, {
                                wpx: 60
                            }, {
                                wpx: 120
                            }, {
                                wpx: 80
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }, {
                                wpx: 120
                            }],
                        },
                    ),
                },
            };

            // 导出 Excel
            XLSX.writeFile(wb, fileName);
        },
        export_xlsx: function (_advance_record) {
            var init_colm = [{
                title: '编号',
                key: 'order_number',
            }, {
                title: '公司',
                key: 'company_name',
            }, {
                title: '物料',
                key: 'stuff_name',
            }, {
                title: '车号',
                key: 'main_vehicle_number',
            }, {
                title: "创建时间",
                key: 'create_time',
            }, {
                title: "确认时间",
                key: 'confirm_time',
            }, {
                title: "进场时间",
                key: 'enter_time',
            }, {
                title: "出场时间",
                key: 'exit_time',
            }, {
                title: "关闭时间",
                key: 'close_time',
            }, {
                title: "一次称重",
                key: 'p_weight',
            }, {
                title: "一次称重时间",
                key: 'p_time',
            }, {
                title: "二次称重",
                key: 'm_weight',
            }, {
                title: "二次称重时间",
                key: 'm_time',
            }, {
                title: "净重",
                key: 'j_weight',
            }, {
                title: "进厂前净重",
                key: 'enter_weight',
            }, {
                title: "单价",
                key: 'price',
            }, {
                title: "总价",
                key: 'total_cost',
            }, {
                title: "司机",
                key: 'driver_name',
            }, {
                title: "电话",
                key: 'driver_phone',
            }, {
                title: "身份证",
                key: 'driver_id',
            }, ];
            var content = [];
            var record_need_export = this.order_selected;
            if (_advance_record) {
                record_need_export = _advance_record;
            }
            record_need_export.forEach(element => {
                var tmp = {
                    ...element
                };
                tmp.status_details.forEach(single_status => {
                    switch (single_status.step) {
                        case 0:
                            tmp.create_time = single_status.timestamp;
                            break;
                        case 1:
                            tmp.confirm_time = single_status.timestamp;
                            break;
                        case 2:
                            tmp.enter_time = single_status.timestamp;
                            break;
                        case 3:
                            tmp.p_time = single_status.timestamp;
                            break;
                        case 4:
                            tmp.m_time = single_status.timestamp;
                            break;
                        case 5:
                            tmp.exit_time = single_status.timestamp;
                            break;
                        case 100:
                            tmp.close_time = single_status.timestamp;
                            break;
                        default:
                            break;
                    }
                });
                tmp.j_weight = Math.abs(tmp.m_weight - tmp.p_weight).toFixed(2);
                tmp.p_weight = tmp.p_weight.toFixed(2);
                tmp.m_weight = tmp.m_weight.toFixed(2);
                tmp.total_cost = (parseFloat(tmp.j_weight) * tmp.price).toFixed(2);
                content.push(tmp);
            });
            this.exportExcel(init_colm, content);
        },
    },
    beforeMount: function () {
        var vue_this = this;
        this.init_statistics();
        vue_this.$call_remote_process("vehicle_management", "get_all_vehicle", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
            resp.forEach((element, index) => {
                vue_this.$set(vue_this.vehicle_for_select, index, element);
            });
        });
        vue_this.$call_remote_process("system_management", "get_domain_name", []).then(function (resp) {
            vue_this.domain_name = resp;
        });
    },
}
</script>

<style scoped>
.single_vehicle_show {
    border: 1px solid black;
    margin-top: 10px;
}

.all_vehicle_select_show {
    max-height: 400px;
    overflow: auto;
}

.step_title {
    font-size: 10px;
}

.step_description_name {
    font-size: 8px;
    color: green;
}

.vehicle_order_center_show {
    height: 88vh;
    overflow: auto;
}

.step_description_time {
    font-size: 6px;
    color: red;
}
</style>
