const express = require('express');
const app = express();
const port = 9098;
const thrift = require('thrift');
const zh_rpc_open_api= require('./gen_code/open_api')
const zh_rpc_vehicle_order_center = require('./gen_code/vehicle_order_center')
const XLSX = require('xlsx');
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

app.post('/zh_rest/vehicle_order/get', async (req, res) =>{
    var ret = { err_msg: '无权限' };

    try {
        var resp = await request_rpc('vehicle_order_center', 'driver_get_order', [req.query.phone, req.query.plate]);
        if (resp) {
            const Int64 = require('node-int64');
            console.log(resp.basic_info.id);
            var id = new Int64(resp.basic_info.id.buffer);
            console.log(id);
            ret = { basic_info: { id: id.toNumber(),driver_id:resp.basic_info.driver_id } };
        }
    } catch (error) {
        ret.err_msg = error;
        if (error.msg) {
            ret.err_msg = error.msg;
        }
    }

    res.send(ret);
});

app.post('/zh_rest/vehicle_order/del', async (req, res) => {
    var ret = { err_msg: '无权限' };
    try {
        var ssid = req.query.zh_ssid;
        console.log(ssid);
        console.log(req.body);
        var resp = await request_rpc('vehicle_order_center', 'cancel_vehicle_order', [ssid, req.body, true]);
        if (resp) {
            ret.err_msg = '';
        }
    } catch (error) {
        ret.err_msg = error;
        if (error.msg) {
            ret.err_msg = error.msg;
        }
    }

    res.send(ret);
});

app.post('/zh_rest/vehicle_order/add', async (req, res) => {
    var ret = { err_msg: '无权限' };
    try {
        var ssid = req.query.zh_ssid;
        var resp = await request_rpc('vehicle_order_center', 'create_vehicle_order', [ssid, req.body, true]);
        if (resp) {
            ret.err_msg = '';
        }
    } catch (error) {
        ret.err_msg = error;
        if (error.msg) {
            ret.err_msg = error.msg;
        }
    }

    res.send(ret);
});
app.post('/zh_rest/order_register/add', async (req, res) => {
    var ret = { err_msg: '无权限' };
    try {
        var resp = await request_rpc('vehicle_order_center', 'driver_check_in', [req.body.id, false, req.body.driver_id, req.body.max_load]);
        if (resp) {
            ret.err_msg = '';
        }
    } catch (error) {
        ret.err_msg = error;
        if (error.msg) {
            ret.err_msg = error.msg;
        }

    }
    res.send(ret);
});
app.post('/zh_rest/order_register/get', async (req, res) => {
    var ret = { err_msg: '无权限' };

    try {
        var resp = await request_rpc('vehicle_order_center', 'driver_get_order', [req.query.phone, req.query.plate]);
        if (resp) {
            console.log(resp);
            const Int64 = require('node-int64');
            var wait_count = new Int64(resp.wait_count.buffer);
            ret = {
                wait_count: wait_count.toNumber(),
                checkin_time: resp.checkin_time,
                enter_location: '',
            };
            console.log(ret);
        }
    } catch (error) {
        console.log(error);
        ret.err_msg = error;
        if (error.msg) {
            ret.err_msg = error.msg;
        }
    }

    res.send(ret);
});
app.post('/zh_rest/order_register/del', async (req, res) => {
    var ret = { err_msg: '无权限' };
    try {
        var resp = await request_rpc('vehicle_order_center', 'driver_check_in', [req.body.id, true]);
        if (resp) {
            ret.err_msg = '';
        }
    } catch (error) {
        ret.err_msg = error;
        if (error.msg) {
            ret.err_msg = error.msg;
        }

    }
    res.send(ret);
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
function calc_test(a, b) {
    return a + b;
}
function exportExcel(headers, data, fileName = '记录表.xlsx') {
    const _headers = headers
        .map((item, i) => Object.assign({}, {
            key: item.key,
            title: item.title,
            position: String.fromCharCode(65 + i) + 1
        }))
        .reduce((prev, next) => Object.assign({}, prev, {
            [next.position]: {
                key: next.key,
                v: next.title
            }
        }), {});

    const _data = data
        .map((item, i) => headers.map((key, j) => Object.assign({}, {
            content: item[key.key],
            position: String.fromCharCode(65 + j) + (i + 2)
        })))
        // 对刚才的结果进行降维处理（二维数组变成一维数组）
        .reduce((prev, next) => prev.concat(next))
        // 转换成 worksheet 需要的结构
        .reduce((prev, next) => Object.assign({}, prev, {
            [next.position]: {
                v: next.content
            }
        }), {});

    // 合并 headers 和 data
    const output = Object.assign({}, _headers, _data);
    // 获取所有单元格的位置
    const outputPos = Object.keys(output);
    // 计算出范围 ,["A1",..., "H2"]
    const ref = `${outputPos[0]}:${outputPos[outputPos.length - 1]}`;

    // 构建 workbook 对象
    const wb = {
        SheetNames: ['mySheet'],
        Sheets: {
            mySheet: Object.assign({},
                output, {
                '!ref': ref,
                '!cols': [{
                    wpx: 80
                }, {
                    wpx: 180
                }, {
                    wpx: 70
                }, {
                    wpx: 120
                }, {
                    wpx: 120
                }, {
                    wpx: 120
                }, {
                    wpx: 120
                }, {
                    wpx: 120
                }, {
                    wpx: 60
                }, {
                    wpx: 120
                }, {
                    wpx: 60
                }, {
                    wpx: 120
                }, {
                    wpx: 80
                }, {
                    wpx: 120
                }, {
                    wpx: 120
                }, {
                    wpx: 120
                }, {
                    wpx: 120
                }, {
                    wpx: 120
                }, {
                    wpx: 120
                }],
            },
            ),
        },
    };

    // 导出 Excel
    XLSX.writeFile(wb, fileName);
};
function pri_export_xlsx (_advance_record) {
    var init_colm = [{
        title: '编号',
        key: 'order_number',
    }, {
        title: '公司',
        key: 'company_name',
    }, {
        title: '物料',
        key: 'stuff_name',
    }, {
        title: '车号',
        key: 'main_vehicle_number',
    }, {
        title: '挂车号',
        key: 'behind_vehicle_number',
    },  {
        title: "进场时间",
        key: 'enter_cam_time',
    }, {
        title: "出场时间",
        key: 'exit_cam_time',
    }, {
        title: "一次称重",
        key: 'p_weight',
    }, {
        title: "一次称重时间",
        key: 'p_cam_time',
    }, {
        title: "二次称重",
        key: 'm_weight',
    }, {
        title: "二次称重时间",
        key: 'm_cam_time',
    }, {
        title: "净重",
        key: 'j_weight',
    }, {
        title: "进厂前净重",
        key: 'enter_weight',
    }, {
        title: "单价",
        key: 'price',
    }, {
        title: "总价",
        key: 'total_cost',
    }, {
        title: "司机",
        key: 'driver_name',
    }, {
        title: "电话",
        key: 'driver_phone',
    }, {
        title: "身份证",
        key: 'driver_id',
    }, {
        title: '手动修改备注',
        key: 'p_m_comment'
    }];
    var has_seal_no = false;
    var has_trans_company = false;
    var has_source_dest_name = false;
    var content = [];
    if (_advance_record) {
        record_need_export = _advance_record;
    }
    var total_j_weight = 0;
    record_need_export.forEach(element => {
        var tmp = {
            ...element
        };
        if (element.is_sale) {
            tmp.j_weight = (tmp.m_weight - tmp.p_weight).toFixed(2);
            total_j_weight += tmp.m_weight - tmp.p_weight;
        } else {
            tmp.j_weight = Math.abs(tmp.m_weight - tmp.p_weight).toFixed(2);
            total_j_weight += Math.abs(tmp.m_weight - tmp.p_weight);
        }
        tmp.p_weight = tmp.p_weight.toFixed(2);
        tmp.m_weight = tmp.m_weight.toFixed(2);
        tmp.total_cost = (parseFloat(tmp.j_weight) * tmp.price).toFixed(2);
        if (tmp.seal_no) {
            has_seal_no = true;
        }
        if (tmp.trans_company) {
            has_trans_company = true;
        }
        if (tmp.source_dest_name) {
            has_source_dest_name = true;
        }
        content.push(tmp);
    });
    content.push({
        order_number: '总计净重',
        company_name: total_j_weight.toFixed(2)
    });
    if (has_seal_no) {
        init_colm.push({
            title: '铅封号',
            key: "seal_no"
        })
    }
    if (has_trans_company) {
        init_colm.push({
            title: '承运公司',
            key: "trans_company"
        })
    }
    if (has_source_dest_name) {
        init_colm.push({
            title: '物料来源/流向',
            key: "source_dest_name"
        });
    }
    const fs = require('fs');
    const path = require('path');
    // 生成临时目录
    const tmpDir = fs.mkdtempSync(path.join(require('os').tmpdir(), 'temp-'));
    // 生成临时文件路径
    const tmpFile = path.join(tmpDir, '记录表.csv');
    var file_content = "";
    var header = [];
    init_colm.forEach(element => {
        header.push(element.title);
    });
    file_content += header.join(',') + '\n';
    content.forEach(element => {
        var tmp = [];
        init_colm.forEach(col => {
            tmp.push(element[col.key]);
        });
        file_content += tmp.join(',') + '\n';
    });
    fs.writeFileSync(tmpFile, file_content);
    //exportExcel(init_colm, content, tmpFile);
    return tmpFile;
};
app.get('/zh_rest/advance_export', async (req, res) => {
    var ret = { err_msg: '无权限' };
    var file_path = '';
    try {
        var token = req.query.zh_ssid;
        var begin_date = req.query.begin_date;
        var end_date = req.query.end_date;
        var company = req.query.company;
        var focus_stuff = req.query.stuff_name;
        var b_time = new Date()
        var resp = await request_rpc("vehicle_order_center", "export_order_by_condition", [token, begin_date, end_date, company, focus_stuff]);
        var e_time = new Date()
        console.log('call rpc spend time:' + (e_time - b_time) + 'ms');
        var tmp = [];
        resp.forEach(element => {
            var single_tmp = element.basic_info;
            single_tmp.enter_cam_time = element.enter_time;
            single_tmp.exit_cam_time = element.exit_time;
            single_tmp.p_cam_time = element.p_time;
            single_tmp.m_cam_time = element.m_time;
            tmp.push(single_tmp);
        });
        b_time = new Date()
        file_path = pri_export_xlsx(tmp);
        console.log('file path:' + file_path);
        e_time = new Date()
        console.log('call export xlsx spend time:' + (e_time - b_time) + 'ms');
        if (file_path) {
            ret.err_msg = '';
        }
    } catch (error) {
        ret = { err_msg: error.msg };
    }
    if (ret.err_msg) {

        res.send(ret);
    }
    else {
        res.sendFile(file_path);
    }
});

app.listen(port, () => {
    console.log('rest is runing');
});