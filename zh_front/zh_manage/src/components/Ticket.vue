<template>
<div class="ticket_show">
    <h3 style="text-align: center;">{{oem_name}}</h3>
    <h3 style="text-align: center;">称重单</h3>
    <ticket-item item_name="磅单号" :item_value="cur_vehicle.basic_info.bl_number"></ticket-item>
    <ticket-item item_name="客户" :item_value="cur_vehicle.basic_info.company_name" v-if="cur_vehicle.is_sale"></ticket-item>
    <ticket-item item_name="供应商" :item_value="cur_vehicle.basic_info.company_name" v-else></ticket-item>
    <ticket-item item_name="车牌号" :item_value="cur_vehicle.basic_info.main_vehicle_number"></ticket-item>
    <ticket-item item_name="产品名称" :item_value="cur_vehicle.basic_info.stuff_name"></ticket-item>
    <ticket-item item_name="皮重" :item_value="cur_vehicle.basic_info.p_weight <= cur_vehicle.basic_info.m_weight? cur_vehicle.basic_info.p_weight.toFixed(2):cur_vehicle.basic_info.m_weight"></ticket-item>
    <ticket-item item_name="毛重" :item_value="cur_vehicle.basic_info.p_weight <= cur_vehicle.basic_info.m_weight? cur_vehicle.basic_info.m_weight.toFixed(2):cur_vehicle.basic_info.p_weight"></ticket-item>
    <ticket-item item_name="净重" :item_value="Math.abs(cur_vehicle.basic_info.p_weight - cur_vehicle.basic_info.m_weight).toFixed(2)"></ticket-item>
    <ticket-item item_name="过皮时间" :item_value="cur_vehicle.p_time"></ticket-item>
    <ticket-item item_name="过毛时间" :item_value="cur_vehicle.m_time"></ticket-item>

</div>
</template>

<script>
import TicketItem from "./TicketItem.vue"
export default {
    name: 'Ticket',
    data: function () {
        return {
            cur_vehicle: {},
            oem_name: '',
        };
    },
    components: {
        "ticket-item": TicketItem,
    },
    props: {
        order_number: String,
    },
    watch: {
        order_number: function () {
            this.init_ticket_data();
        }
    },
    methods: {
        init_ticket_data: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_order_detail", [vue_this.$cookies.get("zh_ssid"), vue_this.order_number]).then(function (resp) {
                vue_this.$set(vue_this.cur_vehicle, "basic_info", resp.basic_info);
                vue_this.$set(vue_this.cur_vehicle, "p_time", resp.p_time);
                vue_this.$set(vue_this.cur_vehicle, "m_time", resp.m_time);
            });
        },
    },
    beforeMount: function () {
        var vue_this = this;
        this.init_ticket_data();
        vue_this.$call_remote_process("system_management", "get_oem_name", []).then(function (resp) {
            vue_this.oem_name = resp;
        });
    }
}
</script>

<style>

</style>
