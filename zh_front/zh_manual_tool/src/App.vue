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
                                    <el-radio :label="single_vehicle.vehicle_number" border>{{single_vehicle.vehicle_number}}</el-radio>
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
                            <el-button v-else type="danger" @click="is_scaling = false">停止称重</el-button>
                        </el-main>
                    </el-container>
                </el-aside>
                <el-aside width="800px" class="print_ticket">
                    <el-container>
                        <el-header>磅单打印</el-header>
                        <el-main >
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
                                    <div class="item_in_ticket">{{print_weight_content.company_name}}</div>
                                    <div class="item_in_ticket">{{print_weight_content.vehicle_number}}</div>
                                    <div class="item_in_ticket">{{print_weight_content.stuff_name}}</div>
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
                                <el-button type="success" v-print="'#weight_ticket'" >打印</el-button>
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
            adjust_position: false,
            search_key: '',
            all_vehicle: [{
                vehicle_number: '蒙A12345',
                company_name: '中燃宏大',
                stuff_name: 'LNG',
                p_weight: 10.7,
                p_time: '2021-12-12 21:07:21',
                m_weight: 31.8,
                m_time: '2021-12-12 22:10:32'
            }, {
                vehicle_number: '晋L88888',
                company_name: '包头展佳',
                stuff_name: '液氮',
                p_weight: 15.7,
                p_time: '2021-12-24 13:07:21',
                m_weight: 41.8,
                m_time: '2021-12-24 13:54:32'

            }],
            selected_vehicle: '',
            is_scaling: false,
            cur_weight: 23.345,
        };
    },
    computed: {
        print_weight_content: function () {
            var ret = {
                vehicle_number: '未选择',
                company_name: '未选择',
                stuff_name: '未选择',
                p_weight: '未知',
                p_time: '未知',
                m_weight: '未知',
                m_time: '未知',
                j_weight: '未知',
            };
            var tmp = this.all_vehicle.find(element => {
                return element.vehicle_number == this.selected_vehicle;
            });
            if (tmp) {
                ret = {
                    ...tmp
                };
                ret.j_weight = ret.m_weight - ret.p_weight;
                ret.m_weight = this.cur_weight;
            }

            return ret;
        },
        vehicle_need_show: function () {
            var ret = [];
            this.all_vehicle.forEach(element => {
                if (this.search_key.length > 0) {
                    if (PinyinMatch.match(element.vehicle_number, this.search_key)) {
                        ret.push(element);
                    }
                } else {
                    ret.push(element);
                }
            });

            return ret;
        },
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
