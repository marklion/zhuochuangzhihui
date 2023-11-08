const express = require('express');
const app = express();
const port = 8080;
const thrift = require('thrift');
const Int64 = require('node-int64');
const _ = require('lodash')
const showdown = require("showdown")

const zh_rpc_config_management = require('../base/build/lib/node_gen_code/config_management')
const zh_rpc_rbac_center = require('../base/build/lib/node_gen_code/rbac_center')
'use strict';

const fs = require('fs');
const { gen_exp } = require('../base/build/lib/node_gen_code/idl_types');
const json2md = require('json2md');

app.use(express.json()) // for parsing application/json
app.use(express.urlencoded({ extended: true })) // for parsing application/x-www-form-urlencoded
function request_rpc(service_name, process_name, params) {
    var mp = new thrift.Multiplexer();
    var options = {
        transport: thrift.TBufferedTransport,
        protocol: thrift.TJSONProtocol,
        path: "/zh_rpc",
        headers: { "Connection": "close" },
        https: false
    };
    var connection = thrift.createHttpConnection("localhost", 8123, options);
    var client = mp.createClient(service_name, eval("zh_rpc_" + service_name), connection);
    return new Promise(function (resolve, reject) {
        client[process_name].apply(client, params).then(function (resp) {
            resolve(resp);
        }).catch(function (err) {
            reject(err);
        });
    });
}

async function verify_rbac(token, target_moduel, target_resource, is_write) {
    var resp = await request_rpc("rbac_center", "has_permission", [token, target_moduel, target_resource, is_write]);
    return resp;
}

const g_api_permisson = {
    "/api/login": {
        module: '',
        resource: '',
        is_write: false,
        no_need_rabc: true,
        handler: async function (body) {
            var ta = [];
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
                explain:[
                    {
                        name: "name",
                        mean:"供应商或客户名称",
                        type:String,
                    },
                    {
                        name: "is_sale",
                        mean:"类型：客户true，供应商 false",
                        type:Boolean,
                    },
                    {
                        name: "attachment",
                        mean:" 附件",
                        type: String,
                    },
                    {
                        name: "id",
                        mean:" 编号",
                        type: Number,
                    },
                    {
                        name: "code",
                        mean:"自定义编码",
                        type: String,
                    },
                    {
                        name: "admin_name_phone",
                        mean:"联系人姓名电话",
                        type: String,
                    },
                    {
                        name: "balance",
                        mean:"余额",
                        type: Number,
                    },
                    {
                        name: "credit",
                        mean:"授信额度",
                        type: Number,
                    },
                    {
                        name: "follow_stuffs",
                        mean:"关注物料列表",
                        type: Array,
                        explain:[
                            {
                                name:'id',
                                mean:"编号",
                                type:Number,
                            },
                            {
                                name:'stuff_name',
                                mean:"物料名称",
                                type:String,
                            },
                            {
                                name:'inventory',
                                mean:"库存",
                                type:Number,
                            },
                            {
                                name:'need_enter_weight',
                                mean:"是否需要进厂前磅单",
                                type:Boolean,
                            },
                            {
                                name:'price',
                                mean:"统一单价",
                                type:Number,
                            },
                            {
                                name:'expect_weight',
                                mean:"期望净重",
                                type:Number,
                            },
                            {
                                name:'need_manual_scale',
                                mean:"是否开启手动确认重量",
                                type:Boolean,
                            },
                            {
                                name:'min_limit',
                                mean:"最小毛重",
                                type:Number,
                            },
                            {
                                name:'max_limit',
                                mean:"最大毛重",
                                type:Number,
                            },
                            {
                                name:'code',
                                mean:"自定义编码",
                                type:String,
                            },
                            {
                                name:'use_for_white_list',
                                mean:"是否用于白名单",
                                type:Boolean,
                            },
                            {
                                name:'auto_call_count',
                                mean:"场内进车容量",
                                type:Number,
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
}
function api_param_walk_check(api_param_req, input) {
    var ret = "params input wrong";
    if (input) {
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
                var sub_ret = api_param_walk_check(element, _.get(input, element.name));
                if (sub_ret.length > 0) {
                    ret = "." + element.name + sub_ret;
                    break;
                }
            }
        }
        else if (api_param_req.type == Array) {
            for (let index = 0; index < api_param_req.explain.length; index++) {
                const element = api_param_req.explain[index];
                var sub_ret = api_param_walk_check(element, _.get(input[0], element.name));
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
    var ret = "params input wrong";
    var api_p = _.get(api_content, 'help_info.params');
    if (api_p) {
        ret = api_param_walk_check(api_p, body);
    }
    else {
        ret = "";
    }

    return ret;
}
Object.keys(g_api_permisson).forEach(key => {
    var api_content = g_api_permisson[key];
    app.post(key, async (req, res) => {
        var ret = {
            err_msg: '无权限',
        };
        var pc_ret = check_api_param(api_content, req.body);
        if (pc_ret.length == 0) {
            try {
                var verify_ret = true;
                if (!api_content.no_need_rabc) {
                    verify_ret = await verify_rbac(req.headers.token, api_content.module, api_content.resource, api_content.is_write);
                }
                if (verify_ret && api_content.handler) {
                    var resp = await api_content.handler(req.body);
                    ret.result = resp;
                    ret.err_msg = "";
                }
            } catch (error) {
                ret.err_msg = error;
                if (error instanceof gen_exp) {
                    ret.err_msg = error.msg;
                }
            }
        }
        else {
            ret.err_msg = pc_ret;
        }
        var s_ret = JSON.stringify(ret, (key, value) => {
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
    var ret = [];

    var rows = [];
    if (params.name) {
        ret.push({
            p: params.name
        });
    }
    var sub_p = [];
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
    var ret = [];

    if (result.name) {
        ret.push({
            p: result.name
        });
    }
    var sub_p = [];
    if (result.explain) {
        var rows = [];
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
    else
    {
        ret.push({
            table: {
                headers: [ "类型", "描述"],
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
    var out_json = [];
    Object.keys(g_api_permisson).forEach(key => {
        var api = g_api_permisson[key];
        var ali = api.help_info;
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
                var tph = make_param_help(ali.params);
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
                var tph = make_result_help(ali.result)
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
    var html_content = cv.makeHtml(json2md(out_json));
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
});