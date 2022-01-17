<template>
<div class="hk_video_show">
    <div>{{title}}</div>
    <div v-if="video_path">
        <video width="320" height="240" controls>
            <source :src="$remote_file_url + video_path" type="video/mp4">
        </video>
    </div>
    <div v-else>
        <el-button type="primary" @click="get_video">查看</el-button>
    </div>
</div>
</template>

<script>
export default {
    name: 'HkVideo',
    props: {
        nvr_ip: String,
        time_center: Date,
        title:String,
    },
    data: function () {
        return {
            video_path: '',
        };
    },
    methods: {
        convert_date_to_string: function (date) {
            var y = date.getFullYear();
            var m = date.getMonth() + 1;
            m = m < 10 ? ('0' + m) : m;
            var d = date.getDate();
            d = d < 10 ? ('0' + d) : d;
            var h = date.getHours();
            h = h < 10 ? ('0' + h) : h;
            var minute = date.getMinutes();
            minute = minute < 10 ? ('0' + minute) : minute;
            var second = date.getSeconds();
            second = second < 10 ? ('0' + second) : second;
            return y + '-' + m + '-' + d + ' ' + h + ':' + minute + ':' + second;
        },
        get_video: function () {
            var vue_this = this;
            var begin_time = new Date(vue_this.time_center);
            var end_time = new Date(vue_this.time_center);
            begin_time.setSeconds(begin_time.getSeconds() - 15);
            end_time.setSeconds(end_time.getSeconds() + 15);
            var nvr_ip = vue_this.nvr_ip.split(":")[0];
            var nvr_channel = vue_this.nvr_ip.split(":")[1];
            vue_this.$call_remote_process("open_api", "get_video", [nvr_ip, parseInt(nvr_channel) + 32, vue_this.convert_date_to_string(begin_time), vue_this.convert_date_to_string(end_time)]).then(function (resp) {
                vue_this.video_path = resp;
            });
        },
    },
}
</script>

<style scoped>
.hk_video_show {
    border: 1px solid black;
}
</style>
