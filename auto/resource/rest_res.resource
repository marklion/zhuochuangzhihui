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
    [Arguments]  ${url}  ${req_dic}=
    ${header}  Create Dictionary  token=${token}
    ${emp_json}  Create Dictionary
    ${req_len}  Get Length  ${req_dic}
    ${req_json}  Set Variable If  ${req_len} == 0  ${emp_json}  ${req_dic}
    ${resp}  POST  url=${BASE_URL}${url}  json=${req_json}  headers=${header}
    RETURN  ${resp}
Auth Api With Max Pri
    IF  $token == ''
        ${user_phone}  ${user_pwd}  Get Max Pri User
        ${login_data}  Create Dictionary  phone=${user_phone}  pwd=${user_pwd}
        ${login_resp}  POST to Server Try  /login  ${login_data}
        ${login_msg}  Set Variable  ${login_resp.json()}[err_msg]
        IF  $login_msg != ''
            Create Max Pri User
            ${login_resp}  POST to Server Try  /login  ${login_data}
        END
        ${token_resp}  Set Variable  ${login_resp.json()}[result]
        Set Suite Variable  ${token}  ${token_resp}
    END
POST to Server Success
    [Arguments]  ${url}  ${req_dic}=
    Auth Api With Max Pri
    ${resp}  POST to Server Try  ${url}  ${req_dic}
    Should Be Empty  ${resp.json()}[err_msg]
    RETURN  ${resp.json()}[result]
POST to Server Failure
    Auth Api With Max Pri
    [Arguments]  ${url}  ${req_dic}=
    ${resp}  POST to Server Try  ${url}  ${req_dic}
    Should Not Be Empty  ${resp.json()}[err_msg]
    RETURN  ${resp.json()}[err_msg]
Start Mock Service
    Start Process  python auto/library/mock_server.py  shell=True  alias=mock_server_process
    Sleep  1s
Get Last Req
    ${result}  GET  url=http://localhost:5001/check_call
    RETURN  ${result.json()}[req]
Get Last Req Path
    ${result}  GET  url=http://localhost:5001/check_call
    RETURN  ${result.json()}[path]
Stop Mock Service
    Terminate Process  mock_server_process
    ${output}  Get Process Result  mock_server_process  stderr=True
    Log  ${output}
