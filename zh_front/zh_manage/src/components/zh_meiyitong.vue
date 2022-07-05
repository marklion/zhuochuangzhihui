<template>
<div class="zh_meiyitong_show">
    <span>监管平台地址</span>
    <el-input v-model="remote_url"></el-input>
    <span>访问凭证</span>
    <el-input v-model="access_code"></el-input>
    <span>UKEY</span>
    <el-input v-model="ukey"></el-input>
    <span>GPS</span>
    <el-input v-model="gps"></el-input>
    <el-button size="small" type="primary" @click="set_config_data">保存</el-button>
    <el-button size="small" type="success" @click="sync_dest_addr">同步拉运目的地</el-button>

    <div v-for="(single_stuff, index) in all_stuff" :key="index">
        <el-row :gutter="10">
            <el-col :span="6">{{single_stuff.name}}</el-col>
            <el-col :span="6">
                <el-select v-model="single_stuff.stand_name" placeholder="请选择煤种">
                    <el-option v-for="item in stand_product_pool" :key="item.ProductID" :label="item.ProductName" :value="item.ProductName">
                    </el-option>
                </el-select>
            </el-col>
            <el-col :span="6">
                <el-select v-model="single_stuff.cust_name" placeholder="请选择煤类">
                    <el-option v-for="item in cust_product_pool" :key="item.ProductID" :label="item.ProductName" :value="item.ProductName">
                    </el-option>
                </el-select>
            </el-col>
            <el-col :span="3">
                <el-input v-model="single_stuff.price" placeholder="请输入单价"></el-input>
            </el-col>
            <el-col :span="3">
                <el-input v-model="single_stuff.heat_value" placeholder="请输入热值"></el-input>
            </el-col>
        </el-row>
    </div>
    <div>
        <el-button size="small" type="success" @click="set_stuff_info">保存</el-button>
    </div>
</div>
</template>

<script>
export default {
    name: "zh_meiyitong",
    data: function () {
        return {
            remote_url: '',
            access_code: "",
            ukey: "",
            gps: "",
            all_stuff: [],
            stand_product_pool: [],
            cust_product_pool: [],
        };
    },
    methods: {
        init_all_data: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k access_code"]).then(function (resp) {
                vue_this.access_code = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k remote_url"]).then(function (resp) {
                vue_this.remote_url = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k ukey"]).then(function (resp) {
                vue_this.ukey = resp;
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k gps"]).then(function (resp) {
                vue_this.gps = resp;
            });
        },
        set_config_data: async function () {
            var vue_this = this;
            try {
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "set -k access_code " + vue_this.access_code]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "set -k remote_url " + vue_this.remote_url]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "set -k ukey " + vue_this.ukey]);
                await vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "set -k gps " + vue_this.gps]);
                vue_this.init_all_data();
            } catch (error) {
                console.log(error);
            }
        },
        sync_dest_addr: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "fetch_dest"]).then(function (resp) {
                var all_dest = JSON.parse(resp);
                all_dest.forEach(element => {
                    vue_this.$call_remote_process("stuff_management", "add_source_dest", [vue_this.$cookies.get("zh_ssid"), element.DivisionName, false, element.DivisionID]);
                });
            });
        },
        set_stuff_info: function () {
            var vue_this = this;
            vue_this.all_stuff.forEach(element => {
                var found_stand_item = vue_this.stand_product_pool.find(item => {
                    return item.ProductName == element.stand_name;
                });
                if (found_stand_item) {
                    element.stand_id = found_stand_item.ProductID;
                }
                var found_cust_item = vue_this.cust_product_pool.find(item => {
                    return item.ProductName == element.cust_name;
                });
                if (found_cust_item) {
                    element.cust_id = found_cust_item.ProductID;
                }
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "set -k product_map '" + JSON.stringify(vue_this.all_stuff) + "' -o"]).then(function () {
                vue_this.init_stuff_data();
            });
        },
        init_product_pool: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k stand_product"]).then(function (resp) {
                vue_this.stand_product_pool = [];
                JSON.parse(resp).forEach((element, index) => {
                    vue_this.$set(vue_this.stand_product_pool, index, element);
                });
            });
            vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k customer_product"]).then(function (resp) {
                vue_this.cust_product_pool = [];
                JSON.parse(resp).forEach((element, index) => {
                    vue_this.$set(vue_this.cust_product_pool, index, element);
                });
            });
        },
        init_stuff_data: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", "get_all_stuff", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.all_stuff = [];
                resp.forEach((element, index) => {
                    element.stand_name = "";
                    element.stand_id = "";
                    element.cust_name = "";
                    element.cust_id = "";
                    element.price = "";
                    element.heat_value = "";
                    vue_this.$set(vue_this.all_stuff, index, element);
                });
                vue_this.$call_remote_process("plugin_management", "run_plugin_cmd", [vue_this.$cookies.get("zh_ssid"), "zh_meiyitong", "get -k product_map"]).then(function (map_resp) {
                    JSON.parse(map_resp).forEach(element => {
                        var found_item = vue_this.all_stuff.find(item => {
                            return item.name == element.name;
                        });
                        if (found_item) {
                            found_item.stand_name = element.stand_name;
                            found_item.stand_id = element.stand_id;
                            found_item.cust_name = element.cust_name;
                            found_item.cust_id = element.cust_id;
                            found_item.price = element.price;
                            found_item.heat_value = element.heat_value;
                        }
                    });
                });
            });
        },
    },
    beforeMount: function () {
        this.init_all_data();
        this.init_stuff_data();
        this.init_product_pool();
    }
}
</script>

<style>

</style>
