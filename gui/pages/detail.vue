<template>
<view v-if="ready_to_show">
    <u-cell-group :title="'单号:' + order.order_number">
        <u-cell :value="order.is_sale?'销售':'采购'" :label="order.company_name+ '-' + order.stuff_name" :title="order.plate_number + '-' + order.back_plate_number">
        </u-cell>
        <u-cell :label="order.id" :value="order.phone" :title="order.driver_name">
        </u-cell>
    </u-cell-group>
    <u-cell-group :title="status_string">
        <u-cell title="排队信息" :label="order.reg_info.operator_time" :value="order.reg_info.operator_name||'未操作'"></u-cell>
        <u-cell title="叫号信息" :label="order.call_info.operator_time" :value="order.call_info.operator_name||'未操作'"></u-cell>
        <u-cell title="确认信息" :label="order.confirm_info.operator_time" :value="order.confirm_info.operator_name||'未操作'"></u-cell>
    </u-cell-group>
    <u-cell-group :title="'净重:' + ((order.p_weight==0 || order.m_weight==0)?'未知':Math.abs(order.p_weight - order.m_weight).toFixed(2))">
        <u-cell v-if="order.p_time" :title="'一次称重:' + order.p_weight" :label="order.p_time" :value="order.is_sale?'皮重':'毛重'"></u-cell>
        <u-cell v-if="order.m_time" :title="'二次称重:' + order.m_weight" :label="order.m_time" :value="order.is_sale?'毛重':'皮重'"></u-cell>
    </u-cell-group>

    <u-cell-group title="流程节点">
        <u-cell v-for="(single_node,index) in order.history_records" :key="index" :title="single_node.node_name" :label="single_node.occour_time" :value="single_node.node_caller"></u-cell>
    </u-cell-group>
    <u-cell-group title="附件">
        <u-cell v-for="(single_node,index) in order.order_attachs" :key="index" :title="single_node.att_name" :label="single_node.att_path"></u-cell>
    </u-cell-group>
    <u-button type="error" text="关闭订单" @click="confirm_close = true" v-if="order.status == 1"></u-button>
    <u-modal :show="confirm_close" @confirm="close_order" closeOnClickOverlay title="确定要关闭吗?" @close="confirm_close = false"></u-modal>
</view>
</template>

<script>
export default {
    data: function () {
        return {
            order: {},
            confirm_close: false,
            ready_to_show: false,
        }
    },
    onLoad: function () {
        this.init_cur_order();
    },
    onPullDownRefresh: function () {
        this.init_cur_order();
    },
    computed: {
        status_string: function () {
            var ret = '未入场';

            switch (this.order.status) {
                case 2:
                    ret = '已出场';
                    break;
                case 100:
                    ret = '已完成';
                    break;
            }

            return ret;
        },
    },
    methods: {
        close_order: function () {
            this.$send_req('/api/order/del', {
                order_number: this.order.order_number
            }).then(() => {
                this.confirm_close = false;
            }).finally(() => {
                uni.startPullDownRefresh();
            });
        },
        init_cur_order: function () {
            this.ready_to_show = false;
            this.$send_req('/api/order/get', {
                order_number: this.$route.query.order_number
            }).then((res) => {
                console.log(res);
                this.order = res;
                this.ready_to_show = true;
            }).finally(() => {
                uni.stopPullDownRefresh();
            });
        },
    }
}
</script>

<style>

</style>
