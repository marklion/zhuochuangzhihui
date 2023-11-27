*** Settings ***
Resource  ../resource/dev_res.resource
Resource  ../resource/rest_res.resource
Library  DateTime
*** Keywords ***
Check In Order
    [Arguments]  ${order_number}  ${is_check_in}=${True}
    ${check_in_req}  Create Dictionary  order_number=${order_number}  is_check_in=${is_check_in}  opt_name=abc
    POST to Server Success  /order/check_in  ${check_in_req}
Call Order
    [Arguments]  ${order_number}  ${is_call}=${True}
    ${call_req}  Create Dictionary  order_number=${order_number}  is_call=${is_call}  opt_name=abc
    POST to Server Success  /order/call  ${call_req}
Confirm Order
    [Arguments]  ${order_number}  ${is_confirm}=${True}
    ${confirm_req}  Create Dictionary  order_number=${order_number}  is_confirm=${is_confirm}  opt_name=abc
    POST to Server Success  /order/confirm  ${confirm_req}
Set Order Seal No
    [Arguments]  ${order_number}  ${seal_no}
    ${seal_req}  Create Dictionary  order_number=${order_number}  is_confirm=${True}  opt_name=abc  seal_no=${seal_no}
    POST to Server Success  /order/set_seal_no  ${seal_req}
Set Auto Call Count
    [Arguments]  ${count}
    [Teardown]  DisConnect
    Connect ZH
    Run ZH Cmd  rule
    Run ZH Cmd  auto_call_count ${count}
Check Call Push
    [Arguments]  ${plate_no}  ${has_call}=${True}  ${driver_name}=
    ${api_sent}  Get Last Req
    IF  ${has_call}
        Should Be Equal As Strings  ${api_sent}[plateNo]  ${plate_no}
        Should Be Equal As Strings  ${api_sent}[driverName]  ${driver_name}
    ELSE
        Should Not Be Equal As Strings  ${api_sent}[plateNo]  ${plate_no}
    END
Check Weight Push
    [Arguments]  ${plate_no}  ${p_weight}  ${m_weight}
    ${api_sent}  Get Last Req
    Should Be Equal As Strings  ${api_sent}[plateNo]  ${plate_no}
    Should Be Equal As Numbers  ${api_sent}[pWeight]  ${p_weight}
    Should Be Equal As Numbers  ${api_sent}[mWeight]  ${m_weight}
    ${expect_date}  Get Current Date  result_format=%Y%m%d
    Should Contain  ${api_sent}[ticketNo]  WR${expect_date}
Get Ticket Last 4 No
    ${api_sent}  Get Last Req
    ${ticket_no}  Set Variable  ${api_sent}[ticketNo]
    ${last4}  Get Substring  ${ticket_no}  -4
    ${ticket_index}  Convert To Integer  ${last4}
    RETURN  ${ticket_index}
Get Reg Orders
    ${req}  Create Dictionary
    ${search_result}  POST to Server Success  /order/get_registered_order  req_dic=${req}
    RETURN  ${search_result}