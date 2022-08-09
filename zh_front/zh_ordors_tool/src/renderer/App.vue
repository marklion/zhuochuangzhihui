<template>
<div id="app">
    <el-row>
        <el-col :span="20">
            <iframe src="http://58.18.38.116:8811/" style="padding: 0;width:100%;" height="1000px" frameborder="1"></iframe>
        </el-col>
        <el-col :span="4">
            <el-form ref="config" :model="config" label-width="auto">
                <el-form-item label="远端地址">
                    <el-input v-model="config.remote_url"></el-input>
                </el-form-item>
                <el-form-item label="GPS">
                    <el-input v-model="config.gps"></el-input>
                </el-form-item>
                <el-form-item label="key_code">
                    <el-input v-model="config.key_code"></el-input>
                </el-form-item>
                <el-form-item label="key_sid">
                    <el-input v-model="config.key_sid"></el-input>
                </el-form-item>
                <el-form-item>
                    <el-button type="primary" @click="set_config">保存</el-button>
                </el-form-item>
            </el-form>
            <el-descriptions v-if="found_vehicle.main_vehicle_number" :title="found_vehicle.main_vehicle_number" :column="1" size="mini" border>
                <el-descriptions-item label="物料">{{found_vehicle.stuff_name}}</el-descriptions-item>
                <el-descriptions-item label="客户">{{found_vehicle.company_name}}</el-descriptions-item>
                <el-descriptions-item label="皮重">{{found_vehicle.p_weight}}</el-descriptions-item>
                <el-descriptions-item label="毛重">{{found_vehicle.m_weight}}</el-descriptions-item>
                <el-descriptions-item label="净重">{{found_vehicle.j_weight}}</el-descriptions-item>
            </el-descriptions>
        </el-col>
    </el-row>
</div>
</template>

<script>
import ws from 'nodejs-websocket'
export default {
    name: 'zh_ordors_tool',
    data: function () {
        return {
            weight: '0.00',
            scan_result: '',
            config: {
                remote_url: '',
                gps: '',
                key_code: '',
                key_sid: '',
            },
            found_vehicle: {
                main_vehicle_number: '',
                stuff_name: '',
                company_name: '',
                p_weight: '',
                m_weight: '',
                j_weight: '',
            },
        };
    },
    methods: {
        get_config: async function () {
            const idb = require('idb-keyval');
            var config = await idb.get("config");
            if (config) {
                this.config = config;
            }
        },
        set_config: function () {
            var vue_this = this;
            const idb = require('idb-keyval');
            idb.set("config", vue_this.config).then(function () {
                vue_this.get_config();
            });
        },
        search_order: function (_scan_string) {
            var vue_this = this;
            var last_dev = _scan_string.lastIndexOf('/');
            var http_head = _scan_string.lastIndexOf('http://');
            if (http_head != -1 && last_dev != -1) {
                var order_number = _scan_string.substring(last_dev + 1, _scan_string.length);
                vue_this.found_vehicle = {
                    main_vehicle_number: '',
                    stuff_name: '',
                    company_name: '',
                    p_weight: '',
                    m_weight: '',
                    j_weight: '',
                };
                vue_this.$http.get(vue_this.config.remote_url + order_number).then(function (resp) {
                    vue_this.found_vehicle = resp.data;
                    if (vue_this.found_vehicle.m_weight) {
                        vue_this.weight = vue_this.found_vehicle.m_weight;
                    } else {
                        vue_this.weight = '0.00';
                    }
                });
            }
        },
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.get_config();
        const server10 = ws.createServer(connect => {
            setInterval(() => {
                connect.sendText(vue_this.weight);
            }, 1500);
        });
        server10.listen(10010);
        const server13 = ws.createServer(connect => {
            connect.on('text', function () {
                connect.sendText(JSON.stringify({
                    "Data": vue_this.config.key_code,
                    "SID": vue_this.config.key_sid,
                    "Type": "Read",
                    "ErrorInfo": "None"
                }))
            });
        });
        server13.listen(10013);
        const server14 = ws.createServer(connect => {
            connect.on('text', function () {
                connect.sendText(vue_this.config.gps);
            });
        });
        server14.listen(10014);

        document.addEventListener("keydown", function (e) {
            console.log(e);
            if (e.key != 'Shift') {
                if (e.code == 'Enter') {
                    var scan_string = vue_this.scan_result;
                    vue_this.scan_result = '';
                    vue_this.search_order(scan_string);
                } else {
                    vue_this.scan_result = vue_this.scan_result + e.key;
                }

            }
        });
    },
}
</script>

<style scoped>
#app {
    height: 100%;
}
</style>
