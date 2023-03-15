<template>
<div class="system_management_show">
    <el-tabs v-model="activeName">
        <el-tab-pane label="规则和维护" name="system_info">
            <el-divider>派车规则</el-divider>
            <el-switch v-model="auto_confirm" @change="change_auto_confirm" active-text="自动确认" inactive-text="手动确认">
            </el-switch>
            <el-divider direction="vertical"></el-divider>
            <el-switch v-model="need_seal_no" @change="change_need_seal_no" active-text="需要铅封号" inactive-text="无需铅封号">
            </el-switch>
            <div v-if="register_time_config.enabled">
                <el-divider>排号配置</el-divider>
                <el-row :gutter="20">
                    <el-col :span="12">
                        <el-form ref="address_form" :model="address_info" label-width="80px">
                            <el-form-item label="排号最远距离">
                                <el-input v-model="address_info.distance"></el-input>
                            </el-form-item>
                            <el-form-item label="厂区位置">
                                <el-col :span="11">
                                    <el-input v-model="address_info.x" placeholder="经度"></el-input>
                                </el-col>
                                <el-col class="line" :span="2">-</el-col>
                                <el-col :span="11">
                                    <el-input v-model="address_info.y" placeholder="纬度"></el-input>
                                </el-col>
                            </el-form-item>
                            <el-form-item>
                                <el-button type="primary" @click="save_address_info">保存</el-button>
                            </el-form-item>
                        </el-form>
                    </el-col>
                    <el-col :span="12">
                        <el-form ref="time_config_form" :model="register_time_config" label-width="80px">
                            <el-form-item label="过号时间">
                                <el-input v-model="register_time_config.pass_time"></el-input>
                            </el-form-item>
                            <el-form-item label="最短离场排号间隔">
                                <el-input v-model="register_time_config.check_in_time"></el-input>
                            </el-form-item>
                            <el-form-item label="最长离场时间">
                                <el-input v-model="register_time_config.leave_limit"></el-input>
                            </el-form-item>
                            <el-form-item>
                                <el-button type="primary" @click="save_register_info">保存</el-button>
                            </el-form-item>
                        </el-form>

                    </el-col>
                </el-row>
            </div>
            <el-divider>指引图片</el-divider>
            <vue-grid align="stretch" justify="start">
                <vue-cell width="2of12" v-for="(single_img,index) in  all_prompt_img" :key="index">
                    <el-image :src="$remote_file_url + single_img.attachment_path" fit="fill"></el-image>
                    <el-button type="text" @click="remove_promp_img(single_img)">删除</el-button>
                </vue-cell>
                <el-upload :show-file-list="false" accept="image/*" :action="$remote_url + '/upload/'" :limit="1" :on-success="upload_prompt_image">
                    <el-button size="small" type="primary">点击上传</el-button>
                </el-upload>
            </vue-grid>
            <el-divider>系统更新</el-divider>
            <el-row :gutter="10" type="flex" justify="start">
                <el-col :span="18">
                    <div>当前版本: {{current_version}}</div>
                </el-col>
                <el-col :span="3">
                    <el-button size="small" type="danger" @click="reboot_system">重启系统</el-button>
                </el-col>
                <el-col :span="3">
                    <el-upload :action="$remote_url + '/upload/'" :limit="1" :on-success="confirm_update">
                        <el-button size="small" type="primary">系统更新</el-button>
                        <div slot="tip" class="el-upload__tip">点击上传官方发布的升级包</div>
                    </el-upload>
                </el-col>
            </el-row>
        </el-tab-pane>
        <el-tab-pane label="插件配置" name="plugin_conf">
            <vue-grid align="stretch" justify="start">
                <vue-cell v-for="(single_plugin, index) in installed_plugins" :key="index" width="6of12">
                    <el-card>
                        <div slot="header" class="clearfix">
                            <span>{{single_plugin}}</span>
                            <el-button type="text" style="float:right;" @click="uninstall_plugin(single_plugin)">卸载</el-button>
                        </div>
                        <component :is="get_component_from_plugin_name(single_plugin)"></component>
                        <el-col :span="20">
                            <el-divider content-position="left">插件队列</el-divider>
                        </el-col>
                        <el-col :span="4">
                            <el-button type="danger" size="small" @click="pop_event_from_que(single_plugin)">弹出</el-button>
                        </el-col>
                        <plugin-que :plugin_name="single_plugin"></plugin-que>
                    </el-card>

                </vue-cell>
                <vue-cell width="6of12">
                    <el-upload :action="$remote_url + '/upload/'" :limit="1" :on-success="install_plugin" :show-file-list="false">
                        <i class="el-icon-plus avatar-uploader-icon"></i>
                    </el-upload>
                </vue-cell>
            </vue-grid>
            <el-row>
                <el-col :span="20">
                    <el-divider content-position="left">插件队列</el-divider>
                </el-col>
                <el-col :span="4">
                    <el-button type="danger" size="small" @click="pop_plugin_que">弹出</el-button>
                </el-col>
            </el-row>
            <div v-for="(single_item, index) in plugin_que" :key="index">{{single_item}}</div>
        </el-tab-pane>
        <el-tab-pane label="设备配置" name="device_config">
            <el-input type="textarea" :rows="200" v-model="core_config_content">
            </el-input>
            <el-button type="success" @click="set_core_config">保存</el-button>
        </el-tab-pane>
    </el-tabs>
</div>
</template>

<script>
import ItemForSelect from "../components/ItemForSelect.vue"
import {
    VueGrid,
    VueCell
} from 'vue-grd';
import PluginQue from '../components/PluginQue.vue'
export default {
    name: 'SystemManagement',
    components: {
        "item-for-select": ItemForSelect,
        "plugin-que": PluginQue,
        VueGrid,
        VueCell
    },
    data: function () {
        return {
            core_config_content: '',
            plugin_que: [],
            address_info: {
                x: 0,
                y: 0,
                distance: 0,
            },
            register_time_config: {
                pass_time: 0,
                check_in_time: 0,
                leave_limit: 0,
                enabled: false,
            },
            auto_confirm: false,
            need_seal_no: false,
            get_component_from_plugin_name: function (_plugin_name) {
                var comp = require(`../components/${_plugin_name}.vue`);
                return comp.default;
            },
            all_prompt_img: [],
            installed_plugins: [],
            activeName: 'system_info',
            current_version: '',
        };
    },
    methods: {
        pop_event_from_que: function (_plugin_name) {
            var vue_this = this;
            vue_this.$dialog.confirm({
                title: '弹出确认',
                message: '确定要弹出该任务吗？'
            }).then(function () {
                vue_this.$call_remote_process("plugin_management", "pop_event_from_que", [vue_this.$cookies.get("zh_ssid"), _plugin_name]).then(function () {
                    vue_this.init_plugins();
                });
            });
        },
        init_plugin_que: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "go_through_plugin_que", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.plugin_que = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.plugin_que, index, element);
                });
            });
        },
        pop_plugin_que: function () {
            var vue_this = this;
            vue_this.$dialog.confirm({
                title: '弹出确认',
                message: '确定要弹出该任务吗？'
            }).then(function () {
                vue_this.$call_remote_process("vehicle_order_center", "cancel_plugin_que", [vue_this.$cookies.get("zh_ssid")]).then(function () {
                    vue_this.init_plugin_que();
                });
            });
        },
        get_register_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_register_info", []).then(function (resp) {
                vue_this.register_time_config = resp;
            });
        },
        save_register_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "set_register_info", [vue_this.$cookies.get("zh_ssid"), {
                enabled: true,
                pass_time: parseInt(vue_this.register_time_config.pass_time),
                check_in_time: parseInt(vue_this.register_time_config.check_in_time),
                leave_limit: parseInt(vue_this.register_time_config.leave_limit),
            }]).then(function (resp) {
                if (resp) {
                    vue_this.get_register_info();
                }
            });
        },
        save_address_info: function () {
            var vue_this = this;
            var set_arg = {
                x: parseFloat(vue_this.address_info.x),
                y: parseFloat(vue_this.address_info.y),
                distance: parseFloat(vue_this.address_info.distance)
            };
            vue_this.$call_remote_process("system_management", "set_company_address_info", [vue_this.$cookies.get("zh_ssid"), set_arg]).then(function (resp) {
                if (resp) {
                    vue_this.get_company_address_info();
                }
            });
        },
        get_company_address_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_company_address_info", []).then(function (resp) {
                vue_this.address_info = resp;
            });
        },
        remove_promp_img: function (_image) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "delete_prompt_image", [vue_this.$cookies.get("zh_ssid"), _image.id]).then(function (resp) {
                if (resp) {
                    vue_this.init_prompt_img();
                }
            });

        },
        upload_prompt_image: function (resp, file) {
            var real_path = resp.match(/^\/tmp\/.*/gm)[0];
            var file_name_split = file.name.split('.');
            var att_path = real_path + '.' + file_name_split[file_name_split.length - 1];
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "upload_prompt_image", [vue_this.$cookies.get("zh_ssid"), att_path]).then(function (resp) {
                if (resp) {
                    vue_this.init_prompt_img();
                }
            });
        },
        install_plugin(resp, file) {
            var real_path = resp.match(/^\/tmp\/.*/gm)[0];
            var file_name_split = file.name.split('.');
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "install_plugin", [vue_this.$cookies.get("zh_ssid"), file_name_split[0], real_path]).then(function (resp) {
                if (resp) {
                    vue_this.init_plugins();
                }
            });
        },
        uninstall_plugin: function (_plugin) {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "uninstall_plugin", [vue_this.$cookies.get("zh_ssid"), _plugin]).then(function () {
                vue_this.init_plugins();
            })
        },
        reboot_system: function () {
            var vue_this = this;
            this.$confirm('确定要重启吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                this.$call_remote_process("system_management", "reboot_system", [vue_this.$cookies.get("zh_ssid")]).finally(() => {
                    vue_this.$alert('请稍后刷新页面', '正在重启', {
                        confirmButtonText: '确定',
                    });
                });
            });
        },
        confirm_update: function (resp) {
            var vue_this = this;
            var real_path = resp.match(/^\/tmp\/.*/gm)[0];
            this.$confirm('确定要更新吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("system_management", "run_update", [vue_this.$cookies.get("zh_ssid"), real_path]).finally(function () {
                    vue_this.$alert('请稍后刷新页面', '正在更新', {
                        confirmButtonText: '确定',
                    });
                });
            });
        },
        init_plugins: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "get_installed_plugins", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.installed_plugins = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.installed_plugins, index, element);
                });
            });
        },
        init_need_seal_no: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "need_seal_no", []).then(function (resp) {
                if (resp) {
                    vue_this.need_seal_no = true;
                } else {
                    vue_this.need_seal_no = false;
                }
            });
        },
        change_need_seal_no: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "set_need_seal_no", [vue_this.$cookies.get("zh_ssid"), vue_this.need_seal_no]).then(function (resp) {
                if (resp) {
                    vue_this.init_need_seal_no();
                }
            });
        },
        change_auto_confirm: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "set_auto_confirm", [vue_this.$cookies.get("zh_ssid"), vue_this.auto_confirm]);
        },
        init_prompt_img: function () {
            var vue_this = this;

            vue_this.$call_remote_process("system_management", "get_all_prompt_image", []).then(function (resp) {
                vue_this.all_prompt_img = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_prompt_img, index, element);
                });
            });
        },
        init_core_config: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_core_config", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.core_config_content = resp;
            });
        },
        set_core_config: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "set_core_config", [vue_this.$cookies.get("zh_ssid"), vue_this.core_config_content]).then(function () {
                vue_this.init_core_config();
            });
        },
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("system_management", 'current_version', []).then(function (resp) {
            vue_this.current_version = resp;
        });
        this.init_plugins();
        vue_this.$call_remote_process("system_management", "is_auto_confirm", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
            vue_this.auto_confirm = resp;
        });
        this.init_prompt_img();
        this.get_company_address_info();
        this.get_register_info();
        this.init_plugin_que();
        this.init_need_seal_no();
        this.init_core_config();
    },
}
</script>

<style scoped>
.device_config_show {
    border: 1px solid black;
    margin-top: 10px;
    padding: 5px;
}
</style>
