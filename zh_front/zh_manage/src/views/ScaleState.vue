<template>
<div class="scale_state_show">

    <van-nav-bar title="称重状态" @click-right="init_scale_state" right-text="刷新" />
    <van-cell v-for="(single_scale, index) in scale_state" :key="index" :title="single_scale.name" :label="single_scale.cur_status" :value="single_scale.weight_pip">
        <template #right-icon>
            <van-button type="danger" size="small" @click="reset_scale(single_scale.name)">重置</van-button>
        </template>
    </van-cell>
</div>
</template>

<script>
export default {
    name: 'ScaleState',
    data: function () {
        return {
            scale_state: [],
        };
    },
    methods: {
        init_scale_state: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_scale_state", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.scale_state = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.scale_state, index, element);
                });
            });
        },
        reset_scale: function (_scale_name) {
            var vue_this = this;
            vue_this.$dialog.confirm({
                message: '确定要重置' + _scale_name + '的状态吗？',
                title: '提示'
            }).then(() => {
                vue_this.$call_remote_process("system_management", "reset_scale_state", [vue_this.$cookies.get("zh_ssid"), _scale_name]).then(function () {
                    vue_this.init_scale_state();
                });
            });
        },
    },
    beforeMount: function () {
        this.init_scale_state();
    },
}
</script>

<style>

</style>
