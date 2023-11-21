const express = require('express');
const app = express();
const port = 8080;
const thrift = require('thrift');
const Int64 = require('node-int64');
const _ = require('lodash')
const showdown = require("showdown")

const zh_rpc_config_management = require('../base/build/lib/node_gen_code/config_management')
const zh_rpc_rbac_center = require('../base/build/lib/node_gen_code/rbac_center')
const zh_rpc_device_management = require('../base/build/lib/node_gen_code/device_management')
const zh_rpc_order_center = require('../base/build/lib/node_gen_code/order_center')
'use strict';

const fs = require('fs');
const { gen_exp } = require('../base/build/lib/node_gen_code/idl_types');
const json2md = require('json2md');

app.use(express.json()) // for parsing application/json
app.use(express.urlencoded({ extended: true })) // for parsing application/x-www-form-urlencoded
function request_rpc(service_name, process_name, params, one_way = false) {
    let mp = new thrift.Multiplexer();
    let path = "/zh_rpc";
    let remote_port = 8123;
    if (service_name == "device_management") {
        remote_port = 8124;
    }
    let options = {
        transport: thrift.TBufferedTransport,
        protocol: thrift.TJSONProtocol,
        path: path,
        https: false,
    };
    let connection = thrift.createHttpConnection("localhost", remote_port, options);
    let new_write = (data) => {
        var http = require('http');
        var https = require('https');
        var self = connection;
        var opts = self.nodeOptions;
        opts.headers["Content-length"] = data.length;
        if (!opts.headers["Content-Type"])
            opts.headers["Content-Type"] = "application/x-thrift";
        var req = (self.https) ?
            https.request(opts, self.responseCallback) :
            http.request(opts, self.responseCallback);
        req.on('error', function (err) {
            if (err.code === "ECONNRESET") {
                console.log("Timeout occurs");
            }
            else {
                self.emit("error", err);
            }
        });
        req.write(data);
        req.end();
        setTimeout(() => {
            req.destroy();
        }, 2000);
    };
    if (one_way) {
        connection.write = new_write;
    }
    let client = mp.createClient(service_name, eval("zh_rpc_" + service_name), connection);
    return new Promise(function (resolve, reject) {
        client[process_name].apply(client, params).then(function (resp) {
            resolve(resp);
        }).catch(function (err) {
            reject(err);
        });
    });
}

async function verify_rbac(token, target_moduel, target_resource, is_write) {
    let resp = await request_rpc("rbac_center", "has_permission", [token, target_moduel, target_resource, is_write]);
    return resp;
}

const g_api_permisson = {
    "/api/login": {
        module: '',
        resource: '',
        is_write: false,
        no_need_rabc: true,
        handler: async function (body) {
            let ta = [];
            return await request_rpc('rbac_center', 'login', [body.phone, body.pwd]);
        },
        help_info: {
            title: "登录系统",
            describe: "调用其他接口前应该先调用此接口获取 token 并保存好",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "phone",
                        type: String,
                        mean: "user's phone number",
                        have_to: true,
                    },
                    {
                        name: "pwd",
                        type: String,
                        mean: "user's password that handled with md5 hashing",
                        have_to: true,
                    }
                ]
            },
            result: {
                type: String,
                mean: "token for other apis",
            },
        },
    },
    "/api/get_stuff_config": {
        module: 'config',
        resource: 'stuff',
        is_write: false,
        handler: async function () {
            return await request_rpc('config_management', 'get_stuff_config', []);
        },

    },
    "/api/get_contract_config": {
        module: 'config',
        resource: 'contract',
        is_write: false,
        handler: async function () {
            return await request_rpc('config_management', 'get_contract_config', []);
        },
        help_info: {
            title: "获取合同配置",
            describe: "获取所有合同信息",
            result: {
                type: Array,
                mean: " 包含所有合同信息的数组",
                explain: [
                    {
                        name: "name",
                        mean: "供应商或客户名称",
                        type: String,
                    },
                    {
                        name: "is_sale",
                        mean: "类型：客户true，供应商 false",
                        type: Boolean,
                    },
                    {
                        name: "attachment",
                        mean: " 附件",
                        type: String,
                    },
                    {
                        name: "id",
                        mean: " 编号",
                        type: Number,
                    },
                    {
                        name: "code",
                        mean: "自定义编码",
                        type: String,
                    },
                    {
                        name: "admin_name_phone",
                        mean: "联系人姓名电话",
                        type: String,
                    },
                    {
                        name: "balance",
                        mean: "余额",
                        type: Number,
                    },
                    {
                        name: "credit",
                        mean: "授信额度",
                        type: Number,
                    },
                    {
                        name: "follow_stuffs",
                        mean: "关注物料列表",
                        type: Array,
                        explain: [
                            {
                                name: 'id',
                                mean: "编号",
                                type: Number,
                            },
                            {
                                name: 'stuff_name',
                                mean: "物料名称",
                                type: String,
                            },
                            {
                                name: 'inventory',
                                mean: "库存",
                                type: Number,
                            },
                            {
                                name: 'need_enter_weight',
                                mean: "是否需要进厂前磅单",
                                type: Boolean,
                            },
                            {
                                name: 'price',
                                mean: "统一单价",
                                type: Number,
                            },
                            {
                                name: 'expect_weight',
                                mean: "期望净重",
                                type: Number,
                            },
                            {
                                name: 'need_manual_scale',
                                mean: "是否开启手动确认重量",
                                type: Boolean,
                            },
                            {
                                name: 'min_limit',
                                mean: "最小毛重",
                                type: Number,
                            },
                            {
                                name: 'max_limit',
                                mean: "最大毛重",
                                type: Number,
                            },
                            {
                                name: 'code',
                                mean: "自定义编码",
                                type: String,
                            },
                            {
                                name: 'use_for_white_list',
                                mean: "是否用于白名单",
                                type: Boolean,
                            },
                            {
                                name: 'auto_call_count',
                                mean: "场内进车容量",
                                type: Number,
                            },
                        ],
                    },
                ],
            },
        },
    },
    "/api/get_vehicle_config": {
        module: 'config',
        resource: 'vehicle',
        is_write: false,
    },
    "/api/add_stuff_config": {
        module: 'config',
        resource: 'stuff',
        is_write: true,
    },
    "/api/update_stuff_config": {
        module: 'config',
        resource: 'stuff',
        is_write: true,
    },
    "/api/add_contract_config": {
        module: 'config',
        resource: 'contract',
        is_write: true,
        handler: async function (body) {
            return await request_rpc('config_management', 'add_contract', [body]);
        },
    },
    "/api/update_contract_config": {
        module: 'config',
        resource: 'contract',
        is_write: true,
    },
    "/api/add_vehicle_config": {
        module: 'config',
        resource: 'vehicle',
        is_write: true,
    },
    "/api/update_vehicle_config": {
        module: 'config',
        resource: 'vehicle',
        is_write: true,
        help_info: {
            title: "modify vehicle config",
            describe: "modify id specified vehicle config to param",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "p1",
                        type: String,
                        mean: "user's phone number",
                        have_to: true,
                    },
                    {
                        name: "p2",
                        type: Number,
                        mean: "user's password that handled with md5 hashing",
                        have_to: true,
                    },
                    {
                        name: "p3",
                        type: String,
                        mean: "user's password that handled with md5 hashing",
                        have_to: true,
                    }, {
                        name: "p4",
                        type: Array,
                        mean: "user's password that handled with md5 hashing",
                        have_to: false,
                        explain: [
                            {
                                name: "a1",
                                type: String,
                                mean: "adsf",
                                have_to: true,
                            },
                        ],
                    },
                ]
            },
            result: {
                type: String,
                mean: "token for other apis",
            },
        },
    },
    "/api/order/add": {
        module: 'order',
        resource: 'vehicle_order_info',
        is_write: true,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'add_order', [body]);
        },
        help_info: {
            title: "创建派车单",
            describe: "创建后派车单处于未入场状态",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "back_plate_number",
                        type: String,
                        mean: "挂车号",
                        have_to: false,
                    },
                    {
                        name: "company_name",
                        type: String,
                        mean: "派车公司",
                        have_to: false,
                    },
                    {
                        name: "driver_id",
                        type: String,
                        mean: "司机身份证号",
                        have_to: false,
                    },
                    {
                        name: "driver_name",
                        type: String,
                        mean: "司机姓名",
                        have_to: false,
                    },
                    {
                        name: "driver_phone",
                        type: String,
                        mean: "司机电话",
                        have_to: true,
                    },
                    {
                        name: "enter_weight",
                        type: Number,
                        mean: "进厂前净重",
                        have_to: false,
                    },
                    {
                        name: "is_sale",
                        type: Boolean,
                        mean: "是否是销售车辆",
                        have_to: false,
                    },
                    {
                        name: "plate_number",
                        type: String,
                        mean: "车牌号",
                        have_to: true,
                    },
                    {
                        name: "stuff_from",
                        type: String,
                        mean: "物料来源地",
                        have_to: false,
                    },
                    {
                        name: "stuff_name",
                        type: String,
                        mean: "物料名",
                        have_to: false,
                    },
                ]
            },
            result: {
                type: Boolean,
                mean: "是否创建成功",
            },
        },
    },
    "/api/order/del": {
        module: 'order',
        resource: 'vehicle_order_info',
        is_write: true,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'del_order', [body.order_number]);
        },
        help_info: {
            title: "关闭派车单",
            describe: "只能关闭未入场的派车单",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "order_number",
                        type: String,
                        mean: "派车单号",
                        have_to: true,
                    },
                ]
            },
            result: {
                type: Boolean,
                mean: "是否关闭成功",
            },
        },
    },
    "/api/order/search": {
        module: 'order',
        resource: 'vehicle_order_info',
        is_write: false,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'search_order', [body]);
        },
        help_info: {
            title: " 搜索派车单",
            describe: "参数是搜索条件",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "plate_number",
                        type: String,
                        mean: "车牌号",
                        have_to: false,
                    },
                    {
                        name: "driver_phone",
                        type: String,
                        mean: "司机电话",
                        have_to: false,
                    },
                    {
                        name: "company_name",
                        type: String,
                        mean: "派车公司",
                        have_to: false,
                    },
                    {
                        name: "stuff_name",
                        type: String,
                        mean: "货物名称",
                        have_to: false,
                    },
                    {
                        name: "status",
                        type: Number,
                        mean: "状态,0->不过滤",
                        have_to: false,
                    },
                    {
                        name: "begin_time",
                        type: String,
                        mean: " 起始时间",
                        have_to: false,
                    },
                    {
                        name: "end_time",
                        type: String,
                        mean: "结束时间",
                        have_to: false,
                    },
                    {
                        name: "page_no",
                        type: Number,
                        mean: "页码，缺省情况为第一页;每页 20 条数据",
                        have_to: false,
                    },
                    {
                        name: "driver_id",
                        type: String,
                        mean: "司机身份证号",
                        have_to: false,
                    },
                    {
                        name: "exp_status",
                        type: Number,
                        mean: "排除的状态",
                        have_to: false,
                    },
                ]
            },
            result: {
                type: Array,
                mean: "搜索到的派车单信息",
                explain: [
                    {
                        name: 'back_plate_number',
                        type: String,
                        mean: "挂车车牌号"
                    },
                    {
                        name: 'call_info',
                        type: Object,
                        mean: "叫号信息",
                        explain: [
                            {
                                name: "operator_name",
                                type: String,
                                mean: '操作人'
                            },
                            {
                                name: "operator_time",
                                type: String,
                                mean: '操作时间'
                            }
                        ]
                    },
                    {
                        name: 'company_name',
                        type: String,
                        mean: "派车公司"
                    },
                    {
                        name: 'confirm_info',
                        type: String,
                        mean: "确认装卸货信息",
                        explain: [
                            {
                                name: "operator_name",
                                type: String,
                                mean: '操作人'
                            },
                            {
                                name: "operator_time",
                                type: String,
                                mean: '操作时间'
                            }
                        ]
                    },
                    {
                        name: 'driver_id',
                        type: String,
                        mean: "司机身份证号"
                    },
                    {
                        name: 'driver_name',
                        type: String,
                        mean: "司机姓名"
                    },
                    {
                        name: 'driver_phone',
                        type: String,
                        mean: "司机电话"
                    },
                    {
                        name: 'enter_weight',
                        type: String,
                        mean: "进厂前重量"
                    },
                    {
                        name: 'id',
                        type: Number,
                        mean: "编号"
                    },
                    {
                        name: 'is_sale',
                        type: Boolean,
                        mean: "是否用于销售"
                    },
                    {
                        name: 'm_time',
                        type: String,
                        mean: "二次称重时间"
                    },
                    {
                        name: 'm_weight',
                        type: Number,
                        mean: "二次称重重量"
                    },
                    {
                        name: 'order_number',
                        type: String,
                        mean: "派车单号",
                    },
                    {
                        name: 'p_time',
                        type: String,
                        mean: "一次称重时间",
                    },
                    {
                        name: 'p_weight',
                        type: Number,
                        mean: "一次称重重量",
                    },
                    {
                        name: 'plate_number',
                        type: String,
                        mean: " 车牌号",
                    },
                    {
                        name: 'reg_info',
                        type: Object,
                        mean: "排号信息",
                        explain: [
                            {
                                name: "operator_name",
                                type: String,
                                mean: '操作人'
                            },
                            {
                                name: "operator_time",
                                type: String,
                                mean: '操作时间'
                            }
                        ],
                    },
                    {
                        name: 'seal_no',
                        type: String,
                        mean: "铅封号",
                    },
                    {
                        name: 'status',
                        type: Number,
                        mean: "状态：0->无效， 1->未入场, 2->正在执行, 100->已完成",
                    },
                    {
                        name: 'stuff_from',
                        type: String,
                        mean: "物料来源地",
                    },
                    {
                        name: 'stuff_name',
                        type: String,
                        mean: "物料名",
                    },
                    {
                        name: 'order_attachs',
                        type: Array,
                        mean: "派车单附件",
                        explain: [
                            {
                                name: 'att_name',
                                type: String,
                                mean: '附件名',
                            },
                            {
                                name: 'att_path',
                                type: String,
                                mean: '附件路径',
                            },
                            {
                                name: 'id',
                                type: Number,
                                mean: ' 编号',
                            },
                        ],
                    },
                    {
                        name: 'history_records',
                        type: Array,
                        mean: "执行历史节点",
                        explain: [
                            {
                                name: "id",
                                type: Number,
                                mean: '编号'
                            },
                            {
                                name: "node_caller",
                                type: String,
                                mean: '执行者'
                            },
                            {
                                name: "node_name",
                                type: String,
                                mean: '节点名'
                            },
                            {
                                name: "occour_time",
                                type: String,
                                mean: '执行时间'
                            },
                        ],
                    },
                ]
            },
        },
    },
    "/api/order/get": {
        module: 'order',
        resource: 'vehicle_order_info',
        is_write: false,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'get_order', [body.order_number]);
        },
        help_info: {
            title: "查找派车单",
            describe: "参数是搜索条件",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: 'order_number',
                        type: String,
                        mean: "订单号",
                        have_to: true,
                    },
                ]
            },
            result: {
                type: Object,
                mean: "查找到的派车单信息",
                explain: [
                    {
                        name: 'back_plate_number',
                        type: String,
                        mean: "挂车车牌号"
                    },
                    {
                        name: 'call_info',
                        type: Object,
                        mean: "叫号信息",
                        explain: [
                            {
                                name: "operator_name",
                                type: String,
                                mean: '操作人'
                            },
                            {
                                name: "operator_time",
                                type: String,
                                mean: '操作时间'
                            }
                        ]
                    },
                    {
                        name: 'company_name',
                        type: String,
                        mean: "派车公司"
                    },
                    {
                        name: 'confirm_info',
                        type: String,
                        mean: "确认装卸货信息",
                        explain: [
                            {
                                name: "operator_name",
                                type: String,
                                mean: '操作人'
                            },
                            {
                                name: "operator_time",
                                type: String,
                                mean: '操作时间'
                            }
                        ]
                    },
                    {
                        name: 'driver_id',
                        type: String,
                        mean: "司机身份证号"
                    },
                    {
                        name: 'driver_name',
                        type: String,
                        mean: "司机姓名"
                    },
                    {
                        name: 'driver_phone',
                        type: String,
                        mean: "司机电话"
                    },
                    {
                        name: 'enter_weight',
                        type: String,
                        mean: "进厂前重量"
                    },
                    {
                        name: 'id',
                        type: Number,
                        mean: "编号"
                    },
                    {
                        name: 'is_sale',
                        type: Boolean,
                        mean: "是否用于销售"
                    },
                    {
                        name: 'm_time',
                        type: String,
                        mean: "二次称重时间"
                    },
                    {
                        name: 'm_weight',
                        type: Number,
                        mean: "二次称重重量"
                    },
                    {
                        name: 'order_number',
                        type: String,
                        mean: "派车单号",
                    },
                    {
                        name: 'p_time',
                        type: String,
                        mean: "一次称重时间",
                    },
                    {
                        name: 'p_weight',
                        type: Number,
                        mean: "一次称重重量",
                    },
                    {
                        name: 'plate_number',
                        type: String,
                        mean: " 车牌号",
                    },
                    {
                        name: 'reg_info',
                        type: Object,
                        mean: "排号信息",
                        explain: [
                            {
                                name: "operator_name",
                                type: String,
                                mean: '操作人'
                            },
                            {
                                name: "operator_time",
                                type: String,
                                mean: '操作时间'
                            }
                        ],
                    },
                    {
                        name: 'seal_no',
                        type: String,
                        mean: "铅封号",
                    },
                    {
                        name: 'status',
                        type: Number,
                        mean: "状态：0->无效， 1->未入场, 2->正在执行, 100->已完成",
                    },
                    {
                        name: 'stuff_from',
                        type: String,
                        mean: "物料来源地",
                    },
                    {
                        name: 'stuff_name',
                        type: String,
                        mean: "物料名",
                    },
                    {
                        name: 'order_attachs',
                        type: Array,
                        mean: "派车单附件",
                        explain: [
                            {
                                name: 'att_name',
                                type: String,
                                mean: '附件名',
                            },
                            {
                                name: 'att_path',
                                type: String,
                                mean: '附件路径',
                            },
                            {
                                name: 'id',
                                type: Number,
                                mean: ' 编号',
                            },
                        ],
                    },
                    {
                        name: 'history_records',
                        type: Array,
                        mean: "执行历史节点",
                        explain: [
                            {
                                name: "id",
                                type: Number,
                                mean: '编号'
                            },
                            {
                                name: "node_caller",
                                type: String,
                                mean: '执行者'
                            },
                            {
                                name: "node_name",
                                type: String,
                                mean: '节点名'
                            },
                            {
                                name: "occour_time",
                                type: String,
                                mean: '执行时间'
                            },
                        ],
                    },
                ]
            },
        },
    },
    "/api/order/check_in": {
        module: 'order',
        resource: 'vehicle_check_in',
        is_write: true,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'order_check_in', [body.order_number, body.is_check_in, body.opt_name]);
        },
        help_info: {
            title: "车辆排号",
            describe: "未入场时可以排号",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "order_number",
                        type: String,
                        mean: "派车单号",
                        have_to: true,
                    },
                    {
                        name: "is_check_in",
                        type: Boolean,
                        mean: "排号还是取消排号",
                        have_to: true,
                    },
                    {
                        name: "opt_name",
                        type: String,
                        mean: "操作人",
                        have_to: true,
                    },
                ]
            },
            result: {
                type: Boolean,
                mean: "是否排号成功",
            },
        },
    },
    "/api/order/call": {
        module: 'order',
        resource: 'vehicle_check_in',
        is_write: true,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'order_call', [body.order_number, body.is_call, body.opt_name]);
        },
        help_info: {
            title: "叫车进场",
            describe: "只能叫已经排号的车辆进场",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "order_number",
                        type: String,
                        mean: "派车单号",
                        have_to: true,
                    },
                    {
                        name: "is_call",
                        type: Boolean,
                        mean: "叫号还是取消叫号",
                        have_to: true,
                    },
                    {
                        name: "opt_name",
                        type: String,
                        mean: "操作人",
                        have_to: true,
                    },
                ]
            },
            result: {
                type: Boolean,
                mean: "是否叫号成功",
            },
        },
    },
    "/api/order/confirm": {
        module: 'order',
        resource: 'vehicle_check_in',
        is_write: true,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'order_confirm', [body.order_number, body.is_confirm, body.opt_name]);
        },
        help_info: {
            title: "确认装卸货",
            describe: "只能对已经叫号的车辆确认装卸货",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "order_number",
                        type: String,
                        mean: "派车单号",
                        have_to: true,
                    },
                    {
                        name: "is_confirm",
                        type: Boolean,
                        mean: "确认还是取消确认",
                        have_to: true,
                    },
                    {
                        name: "opt_name",
                        type: String,
                        mean: "操作人",
                        have_to: true,
                    },
                ]
            },
            result: {
                type: Boolean,
                mean: "是否确认成功",
            },
        },
    },
    "/api/order/set_seal_no": {
        module: 'order',
        resource: 'vehicle_check_in',
        is_write: true,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'order_set_seal_no', [body.order_number, body.seal_no]);
        },
        help_info: {
            title: "设置铅封号",
            describe: "缺省情况下铅封号为空",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "order_number",
                        type: String,
                        mean: "派车单号",
                        have_to: true,
                    },
                    {
                        name: "seal_no",
                        type: String,
                        mean: " 铅封号",
                        have_to: true,
                    },
                ]
            },
            result: {
                type: Boolean,
                mean: "是否设置成功",
            },
        },
    },
    "/api/order/push_gate": {
        module: 'order',
        resource: 'vehicle_check_in',
        is_write: true,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'order_push_gate', [body.order_number, body.opt_name]);
        },
        help_info: {
            title: "推送进出信息",
            describe: "第一次推送为进入，第二次为出",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "order_number",
                        type: String,
                        mean: "派车单号",
                        have_to: true,
                    },
                    {
                        name: "opt_name",
                        type: String,
                        mean: "操作人",
                        have_to: true,
                    },
                ]
            },
            result: {
                type: Boolean,
                mean: "是否进出成功",
            },
        },
    },
    "/api/order/push_weight": {
        module: 'order',
        resource: 'vehicle_check_in',
        is_write: true,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('order_center', 'order_push_weight', [body.order_number, body.weight, body.opt_name]);
        },
        help_info: {
            title: "推送重量",
            describe: "根据派车单状态可能记录为皮重或毛重",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "order_number",
                        type: String,
                        mean: "派车单号",
                        have_to: true,
                    },
                    {
                        name: "weight",
                        type: Number,
                        mean: "重量",
                        have_to: true,
                    },
                    {
                        name: "opt_name",
                        type: String,
                        mean: "操作人",
                        have_to: true,
                    },
                ]
            },
            result: {
                type: Boolean,
                mean: "是否记录成功",
            },
        },
    },
    "/api/device_mock/push_plate": {
        module: 'device',
        resource: 'all_device',
        is_write: true,
        no_need_rabc: false,
        handler: async function (body) {
            return await request_rpc('device_management', 'push_plate_read', [body.device_id, body.plate_no], true);
        },
        help_info: {
            title: "车牌触发",
            describe: "用于仿冒车牌触发",
            params: {
                type: Object,
                have_to: true,
                explain: [
                    {
                        name: "device_id",
                        type: Number,
                        mean: "设备id",
                        have_to: true,
                    },
                    {
                        name: "plate_no",
                        type: String,
                        mean: " 车牌号",
                        have_to: true,
                    }
                ]
            },
            result: {
                type: Boolean,
                mean: "默认成功",
            },
        },
    },
}
function api_param_walk_check(api_param_req, input) {
    let ret = "params input wrong";
    if (input != undefined) {
        if (Object.prototype.toString.call(api_param_req.type()) == Object.prototype.toString.call(input)) {
            ret = "";
        }
        else {
            ret = " require " + api_param_req.type.name + " but input " + Object.prototype.toString.call(input);
        }
    }
    else {
        if (!api_param_req.have_to) {
            ret = "";
        }
        else {
            ret = " param missed";
        }
    }

    if (ret.length == 0 && input) {
        if (api_param_req.type == Object) {
            for (let index = 0; index < api_param_req.explain.length; index++) {
                const element = api_param_req.explain[index];
                let sub_ret = api_param_walk_check(element, _.get(input, element.name));
                if (sub_ret.length > 0) {
                    ret = "." + element.name + sub_ret;
                    break;
                }
            }
        }
        else if (api_param_req.type == Array) {
            for (let index = 0; index < api_param_req.explain.length; index++) {
                const element = api_param_req.explain[index];
                let sub_ret = api_param_walk_check(element, _.get(input[0], element.name));
                if (sub_ret.length > 0) {
                    ret = "." + element.name + sub_ret;
                    break;
                }
            }
        }
    }

    return ret;
}
function check_api_param(api_content, body) {
    let ret = "params input wrong";
    let api_p = _.get(api_content, 'help_info.params');
    if (api_p) {
        ret = api_param_walk_check(api_p, body);
    }
    else {
        ret = "";
    }

    return ret;
}
Object.keys(g_api_permisson).forEach(key => {
    let api_content = g_api_permisson[key];
    app.post(key, async (req, res) => {
        let ret = {
            err_msg: '无权限',
        };
        let pc_ret = check_api_param(api_content, req.body);
        if (pc_ret.length == 0) {
            try {
                let verify_ret = true;
                if (!api_content.no_need_rabc) {
                    verify_ret = await verify_rbac(req.headers.token, api_content.module, api_content.resource, api_content.is_write);
                }
                if (verify_ret && api_content.handler) {
                    let resp = await api_content.handler(req.body);
                    ret.result = resp;
                    ret.err_msg = "";
                }
            } catch (error) {
                ret.err_msg = error;
                console.log(error);
                if (error instanceof gen_exp) {
                    ret.err_msg = error.msg;
                }
            }
        }
        else {
            ret.err_msg = pc_ret;
        }
        let s_ret = JSON.stringify(ret, (key, value) => {
            if (value instanceof Int64) {
                return value.toNumber();
            } else {
                return value;
            }
        });
        res.send(JSON.parse(s_ret));
        String.t
    });
});
function make_param_help(params) {
    let ret = [];

    let rows = [];
    if (params.name) {
        ret.push({
            p: params.name
        });
    }
    let sub_p = [];
    params.explain.forEach(pe => {
        rows.push([pe.name, pe.type.name, pe.have_to.toString(), pe.mean]);
        if (pe.type == Object || pe.type == Array) {
            sub_p.push(pe);
        }
    });
    ret.push({
        table: {
            headers: ["param name", "type", "have to", "describe"],
            rows: rows,
        },
    });

    sub_p.forEach(sp => {
        ret = ret.concat(make_param_help(sp));
    });

    return ret;
}

function make_result_help(result) {
    let ret = [];

    if (result.name) {
        ret.push({
            p: result.name
        });
    }
    let sub_p = [];
    if (result.explain) {
        let rows = [];
        result.explain.forEach(pe => {
            rows.push([pe.name, pe.type.name, pe.mean]);
            if (pe.type == Object || pe.type == Array) {
                sub_p.push(pe);
            }
        });
        ret.push({
            table: {
                headers: ["字段", "类型", "描述"],
                rows: rows,
            },
        });
    }
    else {
        ret.push({
            table: {
                headers: ["类型", "描述"],
                rows: [[result.type.name, result.mean]],
            },
        })
    }

    sub_p.forEach(sp => {
        ret = ret.concat(make_result_help(sp));
    });

    return ret;
}


app.get('/api/help', (req, res) => {
    let out_json = [];
    Object.keys(g_api_permisson).forEach(key => {
        let api = g_api_permisson[key];
        let ali = api.help_info;
        if (ali) {
            out_json.push({
                h1: ali.title
            });
            out_json.push({
                code: {
                    content: key
                }
            });
            out_json.push({
                h2: "describe"
            });
            out_json.push({
                p: ali.describe
            });
            out_json.push({
                h2: "params"
            });
            if (ali.params) {
                let tph = make_param_help(ali.params);
                tph.forEach(sp => {
                    out_json.push(sp);
                });
            }
            else {
                out_json.push({
                    p: "无"
                });
            }
            out_json.push({
                h2: "return"
            });
            if (ali.result) {
                let tph = make_result_help(ali.result)
                console.log(JSON.stringify(tph));
                tph.forEach(sp => {
                    out_json.push(sp);
                });
            }
            else {
                out_json.push({
                    p: "无"
                });
            }
        }
    });
    cv = new showdown.Converter({
        tables: true,
        tablesHeaderId: true
    })
    let html_content = cv.makeHtml(json2md(out_json));
    let md_temp_github = `
<!DOCTYPE html>
<html>
<head>
    <meta charset="utf-8">
    <meta name="viewport" content="width=device-width, initial-scale=1">
    <title>接口文档</title>
    <link rel="stylesheet" href="https://cdn.jsdelivr.net/npm/github-markdown-css@4.0.0/github-markdown.min.css">
</head>
<body>
    <article class="markdown-body">
    ${html_content}
    </article>
</body>
</html>
`
    res.send(md_temp_github);
});


app.listen(port, () => {
    console.log('rest is runing');
    request_rpc("device_management", "init_all_set", [], true);
});