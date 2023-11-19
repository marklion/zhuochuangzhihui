*** Settings ***
Resource  ../resource/dev_res.resource
Resource  ../resource/rest_res.resource

*** Keywords ***
Check In Order
    [Arguments]  ${order_number}
    ${check_in_req}  Create Dictionary  order_number=${order_number}  is_check_in=${True}  opt_name=abc
    POST to Server Success  /order/check_in  ${check_in_req}
Call Order
    [Arguments]  ${order_number}
    ${call_req}  Create Dictionary  order_number=${order_number}  is_call=${True}  opt_name=abc
    POST to Server Success  /order/call  ${call_req}
Confirm Order
    [Arguments]  ${order_number}
    ${confirm_req}  Create Dictionary  order_number=${order_number}  is_confirm=${True}  opt_name=abc
    POST to Server Success  /order/confirm  ${confirm_req}
Set Order Seal No
    [Arguments]  ${order_number}  ${seal_no}
    ${seal_req}  Create Dictionary  order_number=${order_number}  is_confirm=${True}  opt_name=abc  seal_no=${seal_no}
    POST to Server Success  /order/set_seal_no  ${seal_req}