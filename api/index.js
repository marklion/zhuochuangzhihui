const express = require('express');
const app = express();
const port = 8080;
const thrift = require('thrift');

const zh_rpc_config_management = require('../base/build/lib/node_gen_code/config_management')
const zh_rpc_rbac_center= require('../base/build/lib/node_gen_code/rbac_center')
'use strict';

const fs = require('fs');

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

app.get('/api/stuff', async (req, res) => {
    ret = {
        err_msg: '无权限',
    }
    try {
        var verify_ret = await verify_rbac(req.headers.token, "config", "stuff", false);
        if (verify_ret) {
            var resp = await request_rpc('config_management', 'get_stuff_config', []);
            ret = resp;
        }
    } catch (error) {
        ret.err_msg = error;
    }
    res.send(ret);
});


app.listen(port, () => {
    console.log('rest is runing');
});