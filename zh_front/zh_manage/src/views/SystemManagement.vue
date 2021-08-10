<template>
<div class="system_management_show">
    <h2>当前版本: {{current_version}}</h2>
</div>
</template>

<script>
export default {
    name: 'SystemManagement',
    data: function () {
        return {
            current_version:'',
        };
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("system_management", 'current_version', []).then(function(resp) {
            vue_this.current_version = resp;
        });
    },
}
</script>

<style>

</style>
