const express = require('express');
const app = express();
const port = 9098;
const thrift = require('thrift');
const zh_rpc_open_api= require('./gen_code/open_api')
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

app.post('/zh_rest/vehicle_come', async (req, res)=>{
    var ret = { err_msg: '无权限' };
    try {
        for (let i in req.body.params.events) {
            var element = req.body.params.events[i];
            if ((element.eventType == 771760130 || element.eventType == 771760133) && element.data.eventCmd == 1) {
                var road_code = element.data.roadwayIndex;
                var plateNo = element.data.plateNo;
                var resp = await request_rpc("open_api", 'vehicle_come', [plateNo, road_code]);
                if (resp) {
                    ret.err_msg = "";
                }
            }
        };
    } catch (error) {
        ret = { err_msg: error.msg };
    }

    res.send(ret);
});

app.listen(port, () => {
    console.log('rest is runing');
});