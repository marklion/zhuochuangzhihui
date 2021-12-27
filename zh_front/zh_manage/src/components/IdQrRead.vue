<template>
<div class="id_qr_read_show">
    <el-card>
        <div>身份证：{{id_result}}</div>
        <div>二维码：{{qr_result}}</div>
        <div slot="header">
            <span>{{device_name}}</span>
            <el-button style="float: right; padding: 3px 3px" type="warning" @click="read_id_qr">检测</el-button>
        </div>
    </el-card>
</div>
</template>

<script>
export default {
    name: 'IdQrRead',
    props: {
        id_reader_ip: String,
        qr_reader_ip: String,
        device_name: String,
    },
    data: function () {
        return {
            id_result: '',
            qr_result: '',
        };
    },
    methods: {
        read_id_qr: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "read_id_no", [vue_this.id_reader_ip]).then(function (resp) {
                if (resp) {
                    vue_this.id_result = resp;
                } else {
                    vue_this.id_result = "无内容";
                }
            });
            vue_this.$call_remote_process("system_management", "read_qr", [vue_this.qr_reader_ip]).then(function (resp) {
                if (resp) {
                    vue_this.qr_result = resp;
                } else {
                    vue_this.qr_result = '无内容';
                }
            });
        },
    },
}
</script>

<style>

</style>
