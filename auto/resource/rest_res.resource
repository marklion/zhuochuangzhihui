*** Settings ***
Library  RequestsLibrary
Library  Process
Resource  cli_res.resource

*** Variables ***
${BASE_URL}  http://localhost:31700/api
${token}
${mock_server_process}

*** Keywords ***
POST to Server Try
    [Arguments]  ${url}  ${req_dic}
    ${header}  Create Dictionary  token=${token}
    ${resp}  POST  url=${BASE_URL}${url}  json=${req_dic}  headers=${header}
    RETURN  ${resp}

POST to Server Success
    [Arguments]  ${url}  ${req_dic}
    ${resp}  POST to Server Try  ${url}  ${req_dic}
    Should Be Empty  ${resp.json()}[err_msg]
    RETURN  ${resp.json()}[result]

POST to Server Failure
    [Arguments]  ${url}  ${req_dic}
    ${resp}  POST to Server Try  ${url}  ${req_dic}
    Should Not Be Empty  ${resp.json()}[err_msg]
    RETURN  ${resp.json()}[err_msg]

Auth Api With Max Pri
    Create Max Pri User
    ${user_phone}  ${user_pwd}  Get Max Pri User
    ${login_data}  Create Dictionary  phone=${user_phone}  pwd=${user_pwd}
    ${token_resp}  POST to Server Success  /login  ${login_data}
    Set Suite Variable  ${token}  ${token_resp}

Add Order Common
    [Arguments]  ${plate_no}  ${driver_phone}  ${company_name}=测试公司  ${stuff_name}=测试物料  ${driver_id}=33xz
    Auth Api With Max Pri
    ${order_data}  Create Dictionary  plate_number=${plate_no}  driver_phone=${driver_phone}  company_name=${company_name}  stuff_name=${stuff_name}  driver_id=${driver_id}
    POST to Server Success  /order/add  ${order_data}

Get Order By Order Number Not Exist
    [Arguments]  ${order_number}
    Auth Api With Max Pri
    ${search_condition}  Create Dictionary  order_number=${order_number}
    POST to Server Failure  /order/get  ${search_condition}

Get Order By Order Number Exist
    [Arguments]  ${order_number}
    Auth Api With Max Pri
    ${search_condition}  Create Dictionary  order_number=${order_number}
    ${search_result}  POST to Server Success  /order/get  ${search_condition}
    RETURN  ${search_result}

Search Order
    [Arguments]  ${plate_no}=  ${driver_phone}=
    Auth Api With Max Pri
    ${search_condition}  Create Dictionary  plate_number=${plate_no}  driver_phone=${driver_phone}  exp_status=${100}
    ${search_result}  POST to Server Success  /order/search  ${search_condition}
    RETURN  ${search_result}

Del Order Try
    [Arguments]  ${order_id}
    Auth Api With Max Pri
    ${del_req}  Create Dictionary  order_number=${order_id}
    POST to Server Try  /order/del  ${del_req}

Start Mock Service
    Start Process  python auto/library/mock_server.py  shell=True  alias=mock_server_process
    Sleep  2s

Get Last Req
    ${result}  GET  url=http://localhost:5001/check_call
    RETURN  ${result.json()}[req]

Stop Mock Service
    Terminate Process  mock_server_process
    ${output}  Get Process Result  mock_server_process  stderr=True
    Log  ${output}
