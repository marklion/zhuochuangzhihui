from flask import Flask, request
import json

app = Flask(__name__)
base_url = '/pa_web/pa_rest'

g_last_req=json.dumps({
    "result": {
        "id": "123456",
        "drvierName": "aaa"
    }
})
g_last_path = ''

@app.route('/check_call', methods=['GET'])
def check_call():
    ret = json.dumps({
        "path":g_last_path,
        "req": g_last_req,
    })
    return ret;

@app.route(base_url+'/call_vehicle', methods=['POST'])
def record_call():
    # 记录请求内容
    global g_last_path
    global g_last_req
    g_last_req = request.get_json()
    g_last_path = request.path
    # 返回空的响应
    return json.dumps({
        "result": "success"
    });

@app.route(base_url+'/push_p', methods=['POST'])
def record_p():
    # 记录请求内容
    global g_last_path
    global g_last_req
    g_last_req = request.get_json()
    g_last_path = request.path
    # 返回空的响应
    return json.dumps({
        "result": "success"
    });

@app.route(base_url+'/push_weight', methods=['POST'])
def record_weight():
    # 记录请求内容
    global g_last_path
    global g_last_req
    g_last_req = request.get_json()
    g_last_path = request.path
    # 返回空的响应
    return json.dumps({
        "result": "success"
    });

@app.route(base_url+'/vehicle_info', methods=['POST'])
def proc_vehicle_info():
    return json.dumps({
        "result": {
            "id": "123456",
            "driverName": "aaa"
        }
    })

if __name__ == '__main__':
    app.run(port=5001, debug=True, host='0.0.0.0')
