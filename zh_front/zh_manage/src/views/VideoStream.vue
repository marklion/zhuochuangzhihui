<template>
<div>
    <el-button size="mini" type="success" icon="el-icon-plus" @click="show_add_video_diag = true">新增</el-button>
    <el-button size="mini" type="danger" @click="stop_video">停止预览</el-button>
    <el-table :data="all_video_param" style="width: 100%" stripe>
        <el-table-column prop="name" label="名称">
        </el-table-column>
        <el-table-column prop="path" label="路径">
        </el-table-column>
        <el-table-column fixed="right" label="操作">
            <template slot-scope="scope">
                <el-button type="warning" size="mini" @click="set_video_path(scope.row.path)">预览</el-button>
                <el-button type="danger" size="mini" @click="del_video_path(scope.row.id)">删除</el-button>
            </template>
        </el-table-column>
    </el-table>

    <vue-flv-player :autoplay="true" :controls="true" ref="myPlayer" source="/live?app=live&stream=home" />
    <el-dialog title="新增通道" :visible.sync="show_add_video_diag" width="60%" @keyup.enter.native="add_video_path">
        <el-form :model="input_video_param" label-width="120px">
            <el-form-item label="名称" prop="name">
                <el-input v-model="input_video_param.name"></el-input>
            </el-form-item>
            <el-form-item label="路径" prop="path">
                <el-input v-model="input_video_param.path"></el-input>
            </el-form-item>
            <el-form-item>
                <el-button type="primary" @click="add_video_path">确认</el-button>
            </el-form-item>
        </el-form>
    </el-dialog>

</div>
</template>

<script>
import Vue from 'vue'

import vueFlvPlayer from 'vue-flv-player'
Vue.use(vueFlvPlayer)
export default {
    name: 'VideoStream',
    data: function () {
        return {
            input_video_param: {
                name: '',
                path: '',
            },
            all_video_param: [],
            show_add_video_diag: false,
            domain_name: '',
        };
    },
    methods: {
        init_video_path: function () {
            var vue_this = this;
            vue_this.$call_remote_process("open_api", "get_all_video_path", []).then(function (resp) {
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_video_param, index, element);
                });
            });
        },
        add_video_path: function () {
            var vue_this = this;
            vue_this.$call_remote_process("open_api", "add_video_path", [vue_this.$cookies.get("zh_ssid"), vue_this.input_video_param]).then(function (resp) {
                if (resp) {
                    vue_this.init_video_path();
                    vue_this.show_add_video_diag = false;
                }
            });
        },
        del_video_path: function (_id) {
            var vue_this = this;
            this.$alert('确定删除吗?', '删除确认', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("open_api", "del_video_path", [vue_this.$cookies.get("zh_ssid"), _id]);
            });
        },
        set_video_path: function (_path) {
            var vue_this = this;
            vue_this.$call_remote_process("open_api", "set_video_path", [vue_this.$cookies.get("zh_ssid"), _path]).then(function (resp) {
                if (resp) {
                    vue_this.$router.go(0);
                }
            })
        },
        stop_video: function () {
            var vue_this = this;
            vue_this.$call_remote_process("open_api", "stop_video", [vue_this.$cookies.get("zh_ssid")]);
        },
    },
    mounted: function () {
        var vue_this = this;
        vue_this.$call_remote_process("system_management", "get_domain_name", []).then(function (resp) {
            vue_this.domain_name = resp;
        });
        this.init_video_path();
    }
}
</script>

<style>

</style>
