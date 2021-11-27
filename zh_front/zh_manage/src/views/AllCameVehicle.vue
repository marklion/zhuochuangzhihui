<template>
<div class="all_came_vehicle_show">
    <van-nav-bar class="nav_bar_show" title="签到车辆" />
    <van-search v-model="search_key" placeholder="输入公司名拼音首字母/车牌号过滤" />
    <van-cell v-for="(single_vehicle, index) in all_vehicle" :key="index" :title="single_vehicle.company_name" :label="single_vehicle.stuff_name" :value="single_vehicle.main_vehicle_number" center>
        <template #right-icon>
            <div style="margin-left:10px;">
                <van-button v-if="!single_vehicle.enter_order" size="small" type="info">叫号</van-button>
                <div v-else>进厂序号：{{single_vehicle.enter_order}}</div>
            </div>
        </template>
    </van-cell>

</div>
</template>

<script>
import Vue from 'vue';
import Vant from 'vant';
import 'vant/lib/index.css';
Vue.use(Vant);
export default {
    name: 'AllCameVehicle',
    data: function () {
        return {
            search_key:'',
            all_vehicle: [{
                main_vehicle_number: '京AB1234',
                company_name: '尔林兔煤矿',
                stuff_name: '动力煤',
                is_sale: false,
                enter_order: '',
            }, {
                main_vehicle_number: '蒙AB1234',
                company_name: '尔林兔煤矿',
                stuff_name: '动力煤',
                is_sale: false,
                enter_order: '12',
            }, {
                main_vehicle_number: '藏BB1234',
                company_name: '沈阳水泥厂',
                stuff_name: '灰渣',
                is_sale: true,
                enter_order: '',
            }, ],
        };
    },
}
</script>

<style scoped>
.nav_bar_show {
    background-color: #43dbba;
}
</style>
