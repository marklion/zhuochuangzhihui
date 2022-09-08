<template>
<div class="plugin_que_show">
    <div v-for="(single_item, index) in que" :key="index">
        {{single_item}}
    </div>
</div>
</template>

<script>
export default {
    name: 'PluginQue',
    data: function () {
        return {
            que: [],
        };
    },
    props: {
        plugin_name: String,
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("plugin_management", "get_que_by_name", [vue_this.$cookies.get("zh_ssid"), vue_this.plugin_name]).then(function (resp) {
            vue_this.que = [];
            resp.forEach((element, index) => {
                vue_this.$set(vue_this.que, index, element);
            });
        });
    },
}
</script>

<style>

</style>
