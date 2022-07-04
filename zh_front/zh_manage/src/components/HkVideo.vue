<template>
<div class="hk_video_show">
    <div>{{title}}</div>
    <div v-if="video_path">
        <video width="320" height="240" controls>
            <source :src="video_path" type="video/mp4">
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

        get_video: function () {
            var vue_this = this;
            var begin_time = new Date(vue_this.time_center);
            var end_time = new Date(vue_this.time_center);
            begin_time.setSeconds(begin_time.getSeconds() - 8);
            end_time.setSeconds(end_time.getSeconds() + 7);
            var nvr_ip = vue_this.nvr_ip.split(":")[0];
            var nvr_channel = vue_this.nvr_ip.split(":")[1];
            vue_this.$call_remote_process("open_api", "get_video", [nvr_ip, parseInt(nvr_channel), vue_this.$make_time_string(begin_time, '-'), vue_this.$make_time_string(end_time, '-')]).then(function (resp) {
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
