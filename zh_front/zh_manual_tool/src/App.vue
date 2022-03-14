<template>
<div class="app_show">
    <el-container>
        <el-header class="title_header">卓创智汇称重客户端</el-header>
        <el-main>
            <el-container>
                <el-aside width="200px" class="vehicle_search">
                    <el-container>
                        <el-header>车辆检索</el-header>
                        <el-main>
                            <el-input v-model="search_key" placeholder="输入部分或全部车牌搜索"></el-input>
                            <el-radio-group v-model="selected_vehicle" size="small">
                                <div v-for="(single_vehicle, index) in vehicle_need_show" :key="index">
                                    <el-radio :label="single_vehicle.plateNo" border>{{single_vehicle.plateNo}}</el-radio>
                                </div>
                            </el-radio-group>
                        </el-main>
                    </el-container>
                </el-aside>
                <el-aside width="400px" class="scale_weight">
                    <el-container>
                        <el-header>称重</el-header>
                        <el-main>
                            <div>
                                <v-input-sevenseg v-model="cur_weight" height="80" digits="7" :buttons="false"></v-input-sevenseg>
                            </div>
                            <el-button type="primary">清零</el-button>
                            <el-button v-if="!is_scaling" type="success" @click="is_scaling = true">开始称重</el-button>
                            <el-button v-else type="danger" @click="record_weight">停止称重</el-button>
                        </el-main>
                    </el-container>
                </el-aside>
                <el-aside width="800px" class="print_ticket">
                    <el-container>
                        <el-header>磅单打印</el-header>
                        <el-main>
                            <div id="weight_ticket" class="print_content_show">
                                <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="350" h="auto" w="auto" :isResizable="true" parentLimitation>
                                    <div>称重单</div>
                                </vue-drag-resize>
                                <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="30" :y="130" h="auto" w="auto" :isResizable="true" parentLimitation>
                                    <div class="item_in_ticket">公司名:</div>
                                    <div class="item_in_ticket">车牌号:</div>
                                    <div class="item_in_ticket">货物名称:</div>
                                    <div class="item_in_ticket">净重:</div>
                                    <div class="item_in_ticket">皮重:</div>
                                    <div class="item_in_ticket">过皮时间:</div>
                                    <div class="item_in_ticket">毛重:</div>
                                    <div class="item_in_ticket">过毛时间:</div>
                                </vue-drag-resize>

                                <vue-drag-resize preventActiveBehavior :isActive="adjust_position" :x="300" :y="130" h="auto" w="auto" :isResizable="true" parentLimitation>
                                    <div class="item_in_ticket">{{print_weight_content.companyName}}</div>
                                    <div class="item_in_ticket">{{print_weight_content.plateNo}}</div>
                                    <div class="item_in_ticket">{{print_weight_content.stuffName}}</div>
                                    <div class="item_in_ticket">{{print_weight_content.j_weight}}</div>
                                    <div class="item_in_ticket">{{print_weight_content.p_weight}}</div>
                                    <div class="item_in_ticket">{{print_weight_content.p_time}}</div>
                                    <div class="item_in_ticket">{{print_weight_content.m_weight}}</div>
                                    <div class="item_in_ticket">{{print_weight_content.m_time}}</div>
                                </vue-drag-resize>
                            </div>
                        </el-main>
                        <el-footer>
                            <div v-if="!adjust_position">
                                <el-button type="primary" @click="adjust_position = true">调整打印位置</el-button>
                                <el-button type="success" v-print="print_obj">打印</el-button>
                            </div>
                            <div v-else>
                                <el-button type="danger" @click="adjust_position = false">完成调整</el-button>
                            </div>
                        </el-footer>
                    </el-container>
                </el-aside>
            </el-container>
        </el-main>
    </el-container>
</div>
</template>

<script>
import PinyinMatch from "pinyin-match"
import Vue from 'vue'
import VueDragResize from 'vue-drag-resize'
import InputSevenseg from 'v-input-sevenseg'
import Print from 'vue-print-nb'
Vue.use(Print);
Vue.component('v-input-sevenseg', InputSevenseg)
Vue.component('vue-drag-resize', VueDragResize)
export default {
    name: 'App',
    data: function () {
        return {
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
            var tmp = this.all_vehicle.find(element => {
                return element.plateNo == _selected;
            });
            if (tmp) {
                idb.get(tmp.id).then(function (resp) {
                    if (resp) {
                        vue_this.stored_weight = resp;
                    }
                });
            } else {
                vue_this.stored_weight = {
                    id: "",
                    p_weight: '未知',
                    p_time: '未知',
                    m_weight: '未知',
                    m_time: '未知',
                };
            }
        },
    },
    computed: {
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
            };
            var tmp = this.all_vehicle.find(element => {
                return element.plateNo == this.selected_vehicle;
            });
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
                }
            }

            return ret;
        },
        vehicle_need_show: function () {
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
    },
    methods: {
        push_weight: function (_vehicle) {
            var vue_this = this;
            const axios = require('axios');
            axios.post(vue_this.remote_path() + "/pa_web/pa_rest/push_weight?token=" + vue_this.$cookies.get("zy_token"), {
                id: _vehicle.id,
                pWeight: _vehicle.p_weight,
                mWeight: _vehicle.m_weight,
                pTime: _vehicle.pTime,
                mTime: _vehicle.mTime,
                jWeight: _vehicle.j_weight,
            }).then(function (resp) {
                console.log(resp);
            }).catch(function (err) {
                console.log(err);
            });
        },
        record_weight: function () {
            var vue_this = this;
            const idb = require('idb-keyval');
            var tmp = this.all_vehicle.find(element => {
                return element.plateNo == this.selected_vehicle;
            });
            if (tmp) {
                idb.get(tmp.id).then(function (resp) {
                    if (resp) {
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
                            vue_this.push_weight(push_req);

                        });
                    } else {
                        new_weight = {
                            ...vue_this.stored_weight
                        };
                        new_weight.p_weight = vue_this.cur_weight;
                        new_weight.p_time = vue_this.$make_time_string(new Date(), "-");
                        new_weight.id = tmp.id;
                        idb.set(tmp.id, new_weight).then(function () {
                            vue_this.stored_weight = new_weight;
                        });
                    }
                })
            }
            vue_this.is_scaling = false;
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
                vue_this.$cookies.set("zy_token", value, -1);
                vue_this.init_cur_plan_data();
            });
        },
        init_cur_plan_data: function () {
            const axios = require('axios').default;
            var vue_this = this;
            axios.get(vue_this.remote_path() + "/pa_web/pa_rest/all_vehicle_info?token=" + vue_this.$cookies.get("zy_token")).then(function (resp) {
                if (resp.data.err_msg != "") {
                    vue_this.init_token();
                } else {
                    vue_this.all_vehicle = [];
                    resp.data.result.forEach((element, index) => {
                        if (element.id.slice(element.id.length - 1, element.id.length) == 'S') {
                            vue_this.$set(vue_this.all_vehicle, index, element);
                        }
                    });
                }
            }).catch(function (err) {
                console.log(err);
            });
        },
    },
    beforeMount: function () {
        this.init_cur_plan_data();
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
}

.scale_weight {
    background-color: rgb(179, 255, 211);
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
    height: 570px;
    width: 770px;
    margin: 15px;
    border: 1px solid;
    position: relative;
}

.item_in_ticket {
    line-height: 48px;
}
</style>
