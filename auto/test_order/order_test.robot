*** Settings ***
Resource  order_opt.resource
Resource  ../test_field/field_opt.resource

*** Test Cases ***
Order Count Test
    [Setup]  Create Many Orders
    [Teardown]  Del Exist Order
    ${req}  Create Dictionary  status=${1}
    ${count}  POST to Server Success  /order/count  req_dic=${req}
    Should Be Equal As Integers  ${count}  100
Req Queue Test
    [Setup]  Config zyzl Push
    [Teardown]  Clear zyzl Config
    ${on}  Add Order Common  plate1  dp1
    Check In Order  ${on}
    Call Order  ${on}
    Del Order  ${on}
    Sleep  6s
    ${rq}  POST to Server Success  /req_que/get
    Length Should Be  ${rq}  1
    ${pop_req}  Create Dictionary  req_id=${rq[0]['id']}
    POST to Server Success  /req_que/pop  ${pop_req}
    ${rq}  POST to Server Success  /req_que/get
    Length Should Be  ${rq}  0

Modify Driver Id Test
    [Teardown]  Del Exist Order
    ${on}  Add Order Common  plate1  dp1
    Change Driver Id  ${on}  1234
    ${new_o}  Get Order By Order Number Exist  order_number=${on}
    Should Be Equal As Strings  ${new_o['driver_id']}  1234

*** Keywords ***
Create Many Orders
    Del Exist Order
    ${pln}  Set Variable  p
    ${dvp}  Set Variable  d
    FOR  ${i}  IN RANGE  100
        Add Order Common  ${pln}${i}  ${dvp}${i}
    END
Config zyzl Push
    Connect ZH
    Run ZH Cmd  rule
    Run ZH Cmd  set_zyzl_plugin http://192.168.0.109:1111 abc
    DisConnect
Clear zyzl Config
    Connect ZH
    Run ZH Cmd  rule
    Run ZH Cmd  clear
    DisConnect