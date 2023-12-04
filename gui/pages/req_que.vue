<template>
<view>
    <u-cell v-for="(single_item, index) in rqs" :key="index" :title="single_item.req_url" :label="single_item.req_body">
        <view slot="value">
            <u-button text="弹出" type="error" size="small" @click="pop_confirm_id = single_item.id"></u-button>
        </view>
    </u-cell>

    <u-modal :show="pop_confirm_id != 0" title="确定要弹出吗?" closeOnClickOverlay @close="pop_confirm_id = 0" @confirm="pop_out">
    </u-modal>
</view>
</template>

<script>
export default {
    data: function () {
        return {
            rqs: [],
            pop_confirm_id: 0,
        };
    },
    methods: {
        fetch_req_que: function () {
            this.rqs = [];
            this.$send_req('/api/req_que/get', {}).then(resp => {
                this.rqs = resp;
            }).finally(() => {
                uni.stopPullDownRefresh();
            })
        },
        pop_out: function () {
            this.$send_req('/api/req_que/pop', {
                req_id: this.pop_confirm_id
            }).then(() => {
                this.pop_confirm_id = 0;
                uni.startPullDownRefresh();
            })
        },
    },

    onPullDownRefresh() {
        this.fetch_req_que();
    },
    onLoad: function () {
        this.fetch_req_que();
    },
}
</script>

<style>

</style>
