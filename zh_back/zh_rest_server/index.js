const express = require('express');
const app = express();
const port = 9098;
const thrift = require('thrift');
const zh_rpc_open_api= require('./gen_code/open_api')
const zh_rpc_vehicle_order_center = require('./gen_code/vehicle_order_center')
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

app.get('/zh_rest/vehicle_info/:order_number', async (req, res) => {
    try {
        var resp = await request_rpc('vehicle_order_center', 'get_order_detail', ['', req.params.order_number]);
        res.send({
            main_vehicle_number: resp.basic_info.main_vehicle_number,
            stuff_name: resp.basic_info.stuff_name,
            company_name: resp.basic_info.company_name,
            p_weight: resp.basic_info.p_weight.toFixed(2),
            m_weight: resp.basic_info.m_weight.toFixed(2),
            j_weight: (resp.basic_info.m_weight - resp.basic_info.p_weight).toFixed(2),
        });
    } catch (error) {
        res.send({ error: error })
    }
});

app.post('/zh_rest/vehicle_event', async (req, res) => {
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
            if ((element.eventType == 771760131 && element.data.eventCmd == 3) || (element.eventType == 771760134 && element.data.eventCmd == 4)) {
                var road_code = element.data.roadwayIndex;
                var plateNo = element.data.plateNo;
                var resp = await request_rpc("open_api", 'vehicle_leave', [plateNo, road_code]);
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