<template>
<div class="app_show">
    <el-container>
        <el-header class="title_header">卓创智汇称重客户端</el-header>
        <el-main>
            <el-container>
                <el-aside width="700px" class="vehicle_search">
                    <el-container>
                        <el-header>今日计划</el-header>
                        <el-main>
                            <el-row>
                                <el-col :span="8">
                                    <el-button type="primary" @click="init_cur_plan_data">刷新</el-button>
                                </el-col>
                                <el-col :span="16">
                                    <div>今日计划车量:{{vehicle_should_arrive}}车</div>
                                    <div>今日总装车量:{{total_vehicle}}车,{{total_count}}吨</div>
                                </el-col>
                            </el-row>
                            <el-input v-model="search_key" placeholder="输入部分或全部车牌搜索"></el-input>
                            <vue-grid align="stretch" justify="start">
                                <vue-cell width="6of12">
                                    <el-divider>等待称重</el-divider>
                                    <el-radio-group v-model="selected_vehicle" size="small">
                                        <div v-for="(single_vehicle, index) in vehicle_need_show" :key="index" :class="{finish_scale_vehicle_show:single_vehicle.m_time}">
                                            <el-radio :label="single_vehicle.id" border>
                                                {{single_vehicle.plateNo}}-{{single_vehicle.driverName}}
                                                -{{single_vehicle.driverPhone}}
                                            </el-radio>
                                        </div>
                                    </el-radio-group>
                                </vue-cell>
                                <vue-cell width="6of12">
                                    <el-divider>已完成</el-divider>
                                    <el-radio-group v-model="selected_vehicle" size="small">
                                        <div v-for="(single_vehicle, index) in vehicle_finish_scale" :key="index" :class="{finish_scale_vehicle_show:single_vehicle.m_time}">
                                            <el-radio :label="single_vehicle.id" border>
                                                {{single_vehicle.plateNo}}-{{single_vehicle.driverName}}
                                                -{{single_vehicle.driverPhone}}
                                            </el-radio>
                                        </div>
                                    </el-radio-group>

                                </vue-cell>
                                <vue-cell width="6of12">
                                    <el-divider>无证件</el-divider>
                                    <div v-for="(single_vehicle, index) in vehicle_has_to_wait_license" :key="index" :class="{finish_scale_vehicle_show:single_vehicle.m_time}">
                                        {{single_vehicle.plateNo}}-{{single_vehicle.driverName}}
                                        -{{single_vehicle.driverPhone}}
                                    </div>
                                </vue-cell>
                                <vue-cell width="6of12">
                                    <el-divider>未付款</el-divider>
                                    <div v-for="(single_vehicle, index) in vehicle_has_to_wait_payed" :key="index" :class="{finish_scale_vehicle_show:single_vehicle.m_time}">
                                        {{single_vehicle.plateNo}}-{{single_vehicle.driverName}}
                                        -{{single_vehicle.driverPhone}}
                                    </div>
                                </vue-cell>

                            </vue-grid>
                        </el-main>
                    </el-container>
                </el-aside>
                <el-aside width="450px" class="scale_weight">
                    <el-container>
                        <el-header>称重</el-header>
                        <el-main>
                            <el-collapse v-model="activeNames">
                                <el-collapse-item title="串口配置" name="1">
                                    <el-form :inline="true" :model="ser_port_config" class="demo-form-inline">
                                        <el-form-item label="串口号">
                                            <el-input v-model="ser_port_config.port_path" placeholder="串口号"></el-input>
                                        </el-form-item>
                                        <el-form-item label="波特率">
                                            <el-input v-model="ser_port_config.bound_rate" placeholder="波特率"></el-input>
                                        </el-form-item>
                                        <el-form-item>
                                            <el-button type="primary" @click="save_port_config">保存</el-button>
                                        </el-form-item>
                                    </el-form>
                                </el-collapse-item>
                                <el-collapse-item title="解析脚本" name="2">
                                    <el-input type="textarea" :rows="50" placeholder="请输入解析函数" v-model="usr_parse_func">
                                    </el-input>
                                    <el-button type="primary" @click="save_parse">保存</el-button>
                                </el-collapse-item>
                                <el-collapse-item title="磅单前缀" name="3">
                                    <el-input placeholder="请输入磅单前缀" v-model="ticket_prev">
                                    </el-input>
                                    <el-button type="primary" @click="save_ticket_prev">保存</el-button>
                                </el-collapse-item>
                            </el-collapse>
                            <div>
                                <v-input-sevenseg v-model="weight_dig_show" height="80" digits="7" :buttons="false"></v-input-sevenseg>
                            </div>
                            <div v-if="focus_vehicle && !focus_vehicle.m_time">
                                <el-button v-if="!is_scaling" type="success" @click="is_scaling = true">开始称重</el-button>
                                <el-button v-else type="danger" @click="record_weight">停止称重</el-button>
                            </div>
                            <el-button type="danger" v-if="could_be_undo" @click="undo_scale">撤销</el-button>
                            <el-divider></el-divider>
                            <vue-grid align="stretch" justify="start">
                                <vue-cell v-for="(single_license, index) in all_licenses" :key="index" width="6of12">
                                    <el-image :preview-src-list="getPrivewImages(index)" style="width: 100px; height: 100px" :src="remote_path() + single_license.attachment_path" fit="fill"></el-image>
                                    <div>到期时间:{{single_license.expire_date}}</div>
                                </vue-cell>
                            </vue-grid>
                        </el-main>
                    </el-container>
                </el-aside>
            </el-container>
            <div width="800px" class="print_ticket">
                <el-container>
                    <el-header>磅单打印</el-header>
                    <el-main>
                        <div id="weight_ticket" class="print_content_show">
                            <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="130" h="auto" w="auto" :isResizable="true" parentLimitation>
                                <div class="ticket_title_show">{{ticket_param.title}}</div>
                            </vue-drag-resize>
                            <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="5" :y="40" h="auto" w="auto" :isResizable="true" parentLimitation>
                                <div>磅单号:{{print_weight_content.ticket_no}}</div>
                            </vue-drag-resize>
                            <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="350" :y="40" h="auto" w="auto" :isResizable="true" parentLimitation>
                                <div>日期:{{print_weight_content.p_time}}-{{print_weight_content.m_time.substr(11, 8)}}</div>
                            </vue-drag-resize>
                            <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="700" :y="40" h="auto" w="auto" :isResizable="true" parentLimitation>
                                <div>单位:{{ticket_param.unit}}</div>
                            </vue-drag-resize>
                            <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="5" :y="60" h="auto" w="auto" :isResizable="true" parentLimitation>
                                <div class="item_in_ticket">客户名称:{{print_weight_content.companyName}}</div>
                                <div class="item_in_ticket">产品名称:{{print_weight_content.stuffName}}</div>
                                <div class="item_in_ticket">车号:{{print_weight_content.plateNo}}</div>
                                <div class="item_in_ticket">挂车号:{{print_weight_content.backPlateNo}}</div>
                                <div class="item_in_ticket">提货人身份证号:{{print_weight_content.driverId}}</div>
                            </vue-drag-resize>
                            <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="425" :y="60" h="auto" w="auto" :isResizable="true" parentLimitation>
                                <div class="item_in_ticket">毛重:{{calc_weight_show( print_weight_content.m_weight)}}</div>
                                <div class="item_in_ticket">皮重:{{calc_weight_show(print_weight_content.p_weight)}}</div>
                                <div class="item_in_ticket">净重:{{calc_weight_show(print_weight_content.j_weight)}}</div>
                                <div class="item_in_ticket">目的地:{{print_weight_content.sale_address}}</div>
                                <div class="item_in_ticket">提货人签字:</div>
                            </vue-drag-resize>
                            <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="5" :y="230" h="auto" w="auto" :isResizable="true" parentLimitation>
                                <div class="ticket_comment_show">
                                    {{ticket_param.comment}}
                                </div>
                            </vue-drag-resize>
                        </div>
                    </el-main>
                    <el-footer>
                        <div v-if="!adjust_position">
                            <el-button type="primary" @click="show_ticket_param_diag = true">修改参数</el-button>
                            <el-button type="success" v-print="print_obj">打印</el-button>
                        </div>
                        <div v-else>
                            <el-button type="danger" @click="adjust_position = false">完成调整</el-button>
                        </div>
                    </el-footer>
                </el-container>
            </div>
        </el-main>
    </el-container>
    <el-dialog title="修改参数" :visible.sync="show_ticket_param_diag" width="60%" @keyup.enter.native="save_ticket_param">
        <el-form :model="ticket_param" ref="edit_ticket_form" label-width="150px">
            <el-form-item label="磅单标题" prop="title">
                <el-input v-model="ticket_param.title" placeholder="请输入磅单标题"></el-input>
            </el-form-item>
            <el-form-item label="底部提示" prop="comment">
                <el-input v-model="ticket_param.comment" placeholder="请输入底部提示"></el-input>
            </el-form-item>
            <el-form-item label="单位" prop="unit">
                <el-input v-model="ticket_param.unit" placeholder="请输入单位"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="save_ticket_param">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>
</div>
</template>

<script>
import PinyinMatch from "pinyin-match"
import Vue from 'vue'
import VueDragResize from 'vue-drag-resize'
import InputSevenseg from 'v-input-sevenseg'
import Print from 'vue-print-nb'
import {
    Loading
} from 'element-ui';
Vue.use(Print);
Vue.component('v-input-sevenseg', InputSevenseg)
Vue.component('vue-drag-resize', VueDragResize)
import {
    VueGrid,
    VueCell
} from 'vue-grd';
export default {
    name: 'App',
    components: {
        VueGrid,
        VueCell
    },
    data: function () {
        return {
            vehicle_should_arrive: 0,
            total_count: 0,
            total_vehicle: 0,
            could_be_undo: false,
            calc_weight_show: function (_weight) {
                var ret = parseFloat(_weight);
                if (this.ticket_param.unit == '千克') {
                    ret *= 1000;
                    ret = ret.toFixed(0);
                } else {
                    ret = ret.toFixed(2);
                }
                return ret;
            },
            show_ticket_param_diag: false,
            ticket_param: {
                title: '',
                comment: '',
                unit: '',
            },
            activeNames: '',
            usr_parse_func: '',
            ticket_prev: '',
            ser_port_config: {
                port_path: '',
                bound_rate: '',
            },
            print_obj: {
                id: "weight_ticket",
                openCallback: function (vue_this) {
                    vue_this.init_cur_plan_data();
                },
            },
            adjust_position: false,
            search_key: '',
            all_vehicle: [],
            selected_vehicle: '',
            is_scaling: false,
            cur_weight: 23.345,
            stored_weight: {
                id: "",
                p_weight: '未知',
                p_time: '未知',
                m_weight: '未知',
                m_time: '未知',
            },
        };
    },
    watch: {
        selected_vehicle: function (_selected) {
            var vue_this = this;
            const idb = require('idb-keyval');
            vue_this.stored_weight = {
                id: "",
                p_weight: '未知',
                p_time: '未知',
                m_weight: '未知',
                m_time: '未知',
            };
            var tmp = this.all_vehicle.find(element => {
                return element.id == _selected;
            });
            vue_this.could_be_undo = false;
            if (tmp) {
                idb.get(tmp.id).then(function (resp) {
                    if (resp) {
                        vue_this.stored_weight = resp;
                        if (resp.p_time != '未知') {
                            vue_this.could_be_undo = true;
                        }
                    }
                });
            }
        },
    },
    computed: {

        weight_dig_show: {
            get() {
                return this.calc_weight_show(this.cur_weight);
            },
            set(_value) {
                console.log(_value);
            },

        },
        all_licenses: function () {
            var ret = [];
            var tmp = this.focus_vehicle;
            if (tmp) {
                ret = tmp.allLicenseInfo;
            }

            return ret;
        },
        print_weight_content: function () {
            var vue_this = this;
            var ret = {
                plateNo: '未选择',
                companyName: '未选择',
                stuff_name: '未选择',
                p_weight: '未知',
                p_time: '未知',
                m_weight: '未知',
                m_time: '未知',
                j_weight: '未知',
                ticket_no: 'XXXX',
                sale_address: '未知',
            };
            var tmp = vue_this.focus_vehicle;
            console.log(tmp);
            if (tmp) {
                ret = {
                    ...tmp
                };
                ret.p_weight = vue_this.stored_weight.p_weight;
                ret.m_weight = vue_this.stored_weight.m_weight;
                ret.p_time = vue_this.stored_weight.p_time;
                ret.m_time = vue_this.stored_weight.m_time;
                if (isNaN(ret.p_weight) || isNaN(ret.m_weight)) {
                    ret.j_weight = "未知";
                } else {
                    ret.j_weight = ret.m_weight - ret.p_weight;
                    ret.j_weight = vue_this.calc_number_weight(ret.j_weight);
                }
                if (vue_this.stored_weight.ticketNo) {
                    ret.ticket_no = vue_this.stored_weight.ticketNo
                }
            }
            console.log(ret);
            return ret;
        },
        vehicle_after_search: function () {
            var ret = [];
            this.all_vehicle.forEach(element => {
                if (this.search_key.length > 0) {
                    if (PinyinMatch.match(element.plateNo, this.search_key)) {
                        ret.push(element);
                    }
                } else {
                    ret.push(element);
                }
            });

            return ret;
        },
        vehicle_need_show: function () {
            var ret = [];
            this.vehicle_after_search.forEach(element => {
                if (element.has_license && element.has_payed && !element.m_time) {
                    ret.push(element);
                }
            });
            return ret;
        },
        vehicle_finish_scale: function () {
            var ret = [];
            this.vehicle_after_search.forEach(element => {
                if (element.m_time) {
                    ret.push(element);
                }
            });

            return ret;
        },
        vehicle_has_to_wait_payed: function () {
            var ret = [];
            this.vehicle_after_search.forEach(element => {
                if (!element.has_payed) {
                    ret.push(element);
                }
            });
            return ret;
        },
        vehicle_has_to_wait_license: function () {
            var ret = [];
            this.vehicle_after_search.forEach(element => {
                if (!element.has_license) {
                    ret.push(element);
                }
            });
            return ret;
        },
        focus_vehicle: function () {
            return this.all_vehicle.find(element => {
                return element.id == this.selected_vehicle;
            });
        },
    },
    methods: {
        undo_scale: async function () {
            const idb = require('idb-keyval');
            const axios = require('axios');
            var vue_this = this;

            var tmp_vehicle = await idb.get(vue_this.focus_vehicle.id);
            if (tmp_vehicle) {
                var status_string = '一次称重(回皮)';
                if (tmp_vehicle.m_time && tmp_vehicle.m_time != '未知') {
                    status_string = '二次称重(过重)';
                }
                vue_this.$confirm('当前车辆 ' + vue_this.focus_vehicle.plateNo + ' 已经完成' + status_string + ',确定要撤销吗', '撤销确认', {
                    confirmButtonText: '确定',
                    cancelButtonText: '取消',
                    type: 'warning'
                }).then(async function () {
                    var undo_idb_func = function () {
                        tmp_vehicle.p_weight = '未知';
                        tmp_vehicle.p_time = '未知';
                        idb.set(tmp_vehicle.id, tmp_vehicle).then(async function () {
                            vue_this.init_cur_plan_data();
                            vue_this.$message('撤销成功');
                        });
                    };
                    vue_this.$prompt('请输入授权码', '提示', {
                        confirmButtonText: '确定',
                        cancelButtonText: '取消',
                    }).then(async ({
                        value
                    }) => {
                        var token_from_idb = await idb.get("zy_token");
                        try {
                            var undo_ret = await axios.post(vue_this.remote_path() + "/pa_rest/undo_vehicle?token=" + token_from_idb, {
                                id: vue_this.focus_vehicle.id,
                                auth_code: value,
                            });
                            if (undo_ret.data.result) {
                                if (vue_this.focus_vehicle.m_time) {
                                    tmp_vehicle.m_time = '';
                                    tmp_vehicle.m_weight = 0;
                                    await idb.set(tmp_vehicle.id, tmp_vehicle)
                                    vue_this.init_cur_plan_data();
                                    vue_this.$message('撤销成功');
                                } else {
                                    undo_idb_func();
                                }
                            } else {
                                vue_this.$message('无法撤销');
                            }
                        } catch (error) {
                            console.log(error);
                        }
                    });
                });
            }
        },
        calc_number_weight: function (_weight) {
            var ret = 0;
            ret = _weight.toFixed(3);

            return ret;
        },
        getPrivewImages: function (_index) {
            var li_array = [];
            this.all_licenses.forEach(element => {
                li_array.push(this.remote_path() + element.attachment_path);
            });
            let tempImgList = [...li_array];
            if (_index == 0) return tempImgList;
            let start = tempImgList.splice(_index);
            let remain = tempImgList.splice(0, _index);
            return start.concat(remain);

        },
        save_ticket_prev: function () {
            var vue_this = this;
            const idb = require('idb-keyval');
            idb.set("ticket_prev", vue_this.ticket_prev).then(function () {
                window.location.reload();
            });
        },
        save_parse: function () {
            var vue_this = this;
            const idb = require('idb-keyval');
            idb.set("parse_script", vue_this.usr_parse_func).then(function () {
                window.location.reload();
            });
        },
        save_port_config: async function () {
            var vue_this = this;

            const idb = require('idb-keyval');
            await idb.set("port_path", vue_this.ser_port_config.port_path);
            await idb.set("bound_rate", vue_this.ser_port_config.bound_rate);
            window.location.reload();
        },
        push_weight: async function (_vehicle) {
            var vue_this = this;
            const axios = require('axios');
            const idb = require('idb-keyval');
            var token_from_idb = await idb.get("zy_token");
            let loadingInstance = Loading.service({
                fullscreen: true
            });
            axios.post(vue_this.remote_path() + "/pa_rest/push_weight?token=" + token_from_idb, {
                id: _vehicle.id,
                pWeight: _vehicle.p_weight,
                mWeight: _vehicle.m_weight,
                pTime: _vehicle.p_time,
                mTime: _vehicle.m_time,
                jWeight: _vehicle.j_weight,
                ticketNo: _vehicle.ticketNo,
            }).then(function (resp) {
                console.log(resp);
                var tmp = vue_this.focus_vehicle;
                tmp.p_weight = _vehicle.p_weight;
                tmp.p_time = _vehicle.p_time;
                tmp.m_weight = _vehicle.m_weight;
                tmp.m_time = _vehicle.m_time;
                tmp.ticketNo = _vehicle.ticketNo;
                idb.set(_vehicle.id, tmp);
                vue_this.$message('称重成功');
            }).catch(function (err) {
                console.log(err);
            }).finally(function () {
                loadingInstance.close();
            });
        },
        generate_ticket_no: async function () {
            const idb = require('idb-keyval');
            var ret = "";
            var date = new Date();
            var y = date.getFullYear();
            var m = date.getMonth() + 1;
            m = m < 10 ? ('0' + m) : m;
            var d = date.getDate();
            d = d < 10 ? ('0' + d) : d;

            ret = y.toString() + m.toString() + d.toString();
            var last_date = await idb.get("last_date");
            var last_no = await idb.get("last_no");
            if (last_date && last_no && last_date.getDay() == date.getDay()) {
                await idb.set("last_no", last_no + 1);
                ret += String(last_no + 1).padStart(4, '0');
            } else {
                await idb.set("last_date", date);
                await idb.set("last_no", 1);
                ret += '1'.padStart(4, '0');
            }
            console.log(ret);
            return ret;
        },
        record_weight: async function () {
            var vue_this = this;
            const idb = require('idb-keyval');
            var tmp = vue_this.focus_vehicle;
            if (tmp) {
                var weight_turn = '一次称重（回皮）';
                var weight_p_turn = await idb.get(tmp.id);
                if (weight_p_turn && weight_p_turn.p_time != '未知') {
                    weight_turn = '二次称重（过重）';
                }
                vue_this.$confirm('确定要完成 ' + tmp.plateNo + '的' + weight_turn + '吗？', "称重提示", {
                    confirmButtonText: '确定',
                    cancelButtonText: '取消',
                    type: 'warning'
                }).then(function () {
                    idb.get(tmp.id).then(async function (resp) {
                        if (resp && resp.p_time != '未知') {
                            var new_weight = {
                                ...resp
                            };
                            new_weight.m_weight = vue_this.cur_weight;
                            new_weight.m_time = vue_this.$make_time_string(new Date(), "-");
                            idb.set(tmp.id, new_weight).then(function () {
                                vue_this.stored_weight = new_weight;
                                var push_req = {
                                    ...vue_this.stored_weight
                                };
                                push_req.j_weight = push_req.m_weight - push_req.p_weight;
                                push_req.j_weight = vue_this.calc_number_weight(push_req.j_weight);
                                vue_this.push_weight(push_req);

                            });
                        } else {
                            new_weight = {
                                ...vue_this.stored_weight
                            };
                            new_weight.p_weight = vue_this.cur_weight;
                            new_weight.p_time = vue_this.$make_time_string(new Date(), "-");
                            new_weight.id = tmp.id;
                            if (!new_weight.ticketNo) {
                                new_weight.ticketNo = await idb.get("ticket_prev");
                                new_weight.ticketNo += await vue_this.generate_ticket_no();
                            }
                            idb.set(tmp.id, new_weight).then(function () {
                                vue_this.stored_weight = new_weight;
                                vue_this.$message('称重成功');
                            });
                            const axios = require('axios');
                            var token_from_idb = await idb.get("zy_token");
                            axios.post(vue_this.remote_path() + "/pa_rest/push_p?token=" + token_from_idb, {
                                id: tmp.id
                            });
                        }
                        vue_this.is_scaling = false;
                    })
                }).catch(function () {
                    vue_this.is_scaling = false;
                });

            }
        },
        remote_path: function () {
            return this.$remote_url;
        },
        init_token: function () {
            var vue_this = this;
            vue_this.$prompt('请输入令牌', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                inputType: 'text',
                inputPattern: /^[\s\S]*.*[^\s][\s\S]*$/,
                inputErrorMessage: '请输入令牌'
            }).then(function ({
                value
            }) {
                const idb = require('idb-keyval');
                idb.set("zy_token", value).then(function () {
                    vue_this.init_cur_plan_data();
                });
            });
        },
        init_cur_plan_data: async function () {
            const axios = require('axios').default;
            var vue_this = this;
            vue_this.selected_vehicle = '';
            vue_this.stored_weight = {
                id: "",
                p_weight: '未知',
                p_time: '未知',
                m_weight: '未知',
                m_time: '未知',
            };
            const idb = require('idb-keyval');
            var token_from_idb = await idb.get("zy_token");
            let loadingInstance = Loading.service({
                fullscreen: true
            });
            axios.get(vue_this.remote_path() + '/pa_rest/company_brief?token=' + token_from_idb).then(function (resp) {
                vue_this.vehicle_should_arrive = resp.data.result.total_vehicle;
            });
            axios.get(vue_this.remote_path() + "/pa_rest/all_vehicle_info?token=" + token_from_idb).then(async function (resp) {
                if (resp.data.err_msg != "") {
                    vue_this.init_token();
                } else {
                    vue_this.all_vehicle = [];
                    resp.data.result.forEach((element, index) => {
                        if (element.id.slice(element.id.length - 1, element.id.length) == 'S') {
                            vue_this.$set(vue_this.all_vehicle, index, element);
                        }
                    });
                    var vehicle_after_scale = await idb.entries();
                    vehicle_after_scale.forEach(element => {
                        console.log(element);
                        if (/^[0-9]*S$/.test(element[0])) {
                            console.log('match');
                            if (element[1].m_time && element[1].m_time != '未知') {
                                if ((new Date().getDate() != new Date(element[1].m_time).getDate())) {
                                    idb.del(element[0]);
                                } else {
                                    vue_this.$set(vue_this.all_vehicle, vue_this.all_vehicle.length, element[1]);
                                    console.log('push to all');
                                    console.log(vue_this.all_vehicle);
                                }
                            }
                        }
                    });
                    vue_this.total_vehicle = 0;
                    vue_this.total_count = 0;
                    var tmp_all_vehicle = [];
                    vue_this.all_vehicle.forEach(element => {
                        if (element.createTime.substr(0, 10) == vue_this.$make_time_string(new Date(), '-').substr(0, 10)) {
                            tmp_all_vehicle.push(element);
                        }
                    });
                    vue_this.all_vehicle = tmp_all_vehicle;
                    vue_this.all_vehicle.forEach(element => {
                        if (element.m_time && new Date(element.m_time).getDate() == new Date().getDate()) {
                            vue_this.total_vehicle += 1;
                            vue_this.total_count += (parseFloat(element.m_weight) - parseFloat(element.p_weight));
                        }
                    });
                    vue_this.total_count = vue_this.calc_number_weight(vue_this.total_count);
                    console.log('all');
                    console.log(vue_this.all_vehicle);

                }
            }).catch(function (err) {
                console.log(err);
            }).finally(function () {
                loadingInstance.close();
            });

        },
        init_ticket_param: async function () {
            const idb = require('idb-keyval');
            var stored_param = await idb.get("ticket_param");
            if (stored_param) {
                this.ticket_param = stored_param;
            } else {
                this.ticket_param = {
                    title: '称重单',
                    comment: '第一联：生产留存    第二联：销售留存    第三联：财务留存     第四联：客户留存    第五联：运输公司留存',
                    unit: '吨',
                };
            }
        },
        save_ticket_param: function () {
            const idb = require('idb-keyval');
            var vue_this = this;
            idb.set("ticket_param", vue_this.ticket_param).then(function () {
                vue_this.init_ticket_param();
                vue_this.show_ticket_param_diag = false;
            });
        },
    },
    beforeMount: async function () {
        this.init_ticket_param();
        this.init_cur_plan_data();
        var vue_this = this;
        const {
            SerialPort
        } = require('serialport')
        const idb = require('idb-keyval');
        var port_path = await idb.get("port_path");
        var bound_rate = await idb.get("bound_rate");
        var usr_parse_func = await idb.get("parse_script");
        this.ser_port_config.port_path = port_path ? port_path : 'COM1';
        this.ser_port_config.bound_rate = port_path ? bound_rate : '9600';
        this.usr_parse_func = usr_parse_func ? usr_parse_func : `function (buff) {
    var ret = 0;
    var last_byte_pos = buff.length - 1;
    var first_byte_pos = 0;
    while (last_byte_pos >= 11)
    {
        first_byte_pos = last_byte_pos - 11;
        if (buff[first_byte_pos] == 0x02 && buff[last_byte_pos] == 0x03)
        {
            var sign_flag = buff[first_byte_pos + 1];
            var ten_flag = (buff[first_byte_pos + 8] - 0x30);
            ret += (buff[first_byte_pos + 2] - 0x30) * (10**5);
            ret += (buff[first_byte_pos + 3] - 0x30) * (10**4);
            ret += (buff[first_byte_pos + 4] - 0x30) * (10**3);
            ret += (buff[first_byte_pos + 5] - 0x30) * (10**2);
            ret += (buff[first_byte_pos + 6] - 0x30) * (10**1);
            ret += (buff[first_byte_pos + 7] - 0x30) * (10**0);
            ret /= 10**(ten_flag);
            if (0x2b != sign_flag)
            {
                ret = 0 - ret;
            }
            break;
        }
        else {
            last_byte_pos--;
        }
    }

    return ret;
};`;
        const port = new SerialPort({
            path: this.ser_port_config.port_path,
            baudRate: parseInt(this.ser_port_config.bound_rate),
        }, function (err) {
            if (err) {
                return console.log('Error: ', err.message)
            }
        })
        port.on("data", function (_data) {
            console.log(_data);
            let parse_func = new Function('return ' + vue_this.usr_parse_func);
            vue_this.cur_weight = parse_func()(_data);
        });
    },
}
</script>

<style scoped>
.title_header {
    font-size: 26px;
    background-color: antiquewhite;
}

.vehicle_search {
    background-color: rgb(192, 200, 158);
    height: 580px;
}

.scale_weight {
    background-color: rgb(179, 255, 211);
    height: 580px;
}

.print_ticket {
    background-color: rgb(190, 221, 241);
}

.el-header,
.el-footer {
    text-align: center;
    padding: 0;
}

.el-aside {
    text-align: center;
    padding: 0;
}

.el-main {
    text-align: center;
    padding: 0;
}

.weight_show {
    color: red;
    font-size: 30px
}

.print_content_show {
    height: 250px;
    width: 770px;
    margin: 15px;
    position: relative;
}

.item_in_ticket {
    line-height: 30px;
    width: 420px;
    text-align: justify;
    border: 1px solid;
}

.ticket_title_show {
    line-height: 40px;
    font-size: 35px;
}

.ticket_comment_show {
    line-height: 10px;
    font-size: 15px;
}

.finish_scale_vehicle_show {
    background-color: rgb(175, 106, 106);
}
</style>
