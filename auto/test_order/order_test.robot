*** Settings ***
Resource  order_opt.resource

*** Test Cases ***
Order Count Test
    [Setup]  Create Many Orders
    [Teardown]  Del Exist Order
    ${req}  Create Dictionary  status=${1}
    ${count}  POST to Server Success  /order/count  req_dic=${req}
    Should Be Equal As Integers  ${count}  100

*** Keywords ***
Create Many Orders
    Del Exist Order
    ${pln}  Set Variable  p
    ${dvp}  Set Variable  d
    FOR  ${i}  IN RANGE  100
        Add Order Common  ${pln}${i}  ${dvp}${i}
    END