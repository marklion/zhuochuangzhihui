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
        device_name: String,
        begin_time: String,
        end_time: String,
        title: String,
        is_enter: Boolean,
    },
    data: function () {
        return {
            video_path: '',
        };
    },
    methods: {

        get_video: function () {
            var vue_this = this;
            var end_time = vue_this.end_time;
            if (!vue_this.end_time)
            {
                var end_time_va = new Date(vue_this.begin_time);
                end_time_va.setSeconds(end_time_va.getSeconds() + 10);
                end_time = vue_this.$make_time_string(end_time_va, '-');
            }
            vue_this.$call_remote_process("system_management", "get_video", [vue_this.device_name, vue_this.is_enter, vue_this.begin_time, end_time, vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
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
