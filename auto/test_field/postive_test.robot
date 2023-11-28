*** Settings ***
Resource  field_opt.resource
Resource  ../test_order/order_opt.resource
Suite Setup  Run Keywords  Config Best Dev  AND  Start Mock Service
Suite Teardown  Run Keywords  Clear Dev Config  AND  Stop Mock Service
*** Variables ***
@{m_pl}  京A12345  京A12346  京A12347  京A12348
@{m_dh}  18811111111  18811111112  18811111113  18811111114
*** Test Cases ***
Check In No Increase
    [Teardown]  Del Exist Order
    @{multi_orders}  Make Multi Info
    Create Mult Order  @{multi_orders}
    ${another_order}  Add Order Common  蒙B11221  19999887761
    Check In Order  ${multi_orders}[0][order_number]
    Sleep  2s
    Check In Order  ${multi_orders}[3][order_number]
    Sleep  2s
    Check In Order  ${multi_orders}[1][order_number]
    Sleep  2s
    Check In Order  ${multi_orders}[2][order_number]
    @{reg_orders}  Get Reg Orders
    ${check_in_count}  Get Length  ${multi_orders}
    Length Should Be  ${reg_orders}  length=${check_in_count}
    ${count_minus_1}  Evaluate  ${check_in_count} - 1
    FOR  ${index}  IN RANGE  ${count_minus_1}
        ${back_index}  Evaluate  ${index} + 1
        ${front_order}  Get From List  ${reg_orders}  ${index}
        ${back_order}  Get From List  ${reg_orders}  ${back_index}
        ${diff}  Evaluate  ${back_order}[reg_no] - ${front_order}[reg_no]
        Should Be Equal As Integers  ${diff}  1
    END

Auto Call When Check In
    [Teardown]  Del Exist Order
    @{multi_orders}  Make Multi Info
    Create Mult Order  @{multi_orders}
    Set Auto Call Count  2
    ${first_order}  Get From List  ${multi_orders}  0
    ${second_order}  Get From List  ${multi_orders}  1
    ${third_order}  Get From List  ${multi_orders}  2
    ${fourth_order}  Get From List  ${multi_orders}  3
    Check In Order  ${first_order}[order_number]
    Sleep  5s
    Check Call Push  ${first_order}[plate_no]
    Check In Order  ${second_order}[order_number]
    Sleep  5s
    Check Call Push  ${second_order}[plate_no]
    Check In Order  ${third_order}[order_number]
    Sleep  5s
    Check Call Push  ${third_order}[plate_no]  has_call=${False}
Auto Call When Cancle Call
    [Teardown]  Del Exist Order
    @{multi_orders}  Make Multi Info
    Create Mult Order  @{multi_orders}
    Set Auto Call Count  3
    ${first_order}  Get From List  ${multi_orders}  0
    ${second_order}  Get From List  ${multi_orders}  1
    ${third_order}  Get From List  ${multi_orders}  2
    ${fourth_order}  Get From List  ${multi_orders}  3
    Check In Order  ${first_order}[order_number]
    Check In Order  ${second_order}[order_number]
    Check In Order  ${third_order}[order_number]
    Check In Order  ${fourth_order}[order_number]
    Sleep  5s
    Check Call Push  ${fourth_order}[plate_no]  has_call=${False}
    Check In Order  ${second_order}[order_number]  is_check_in=${False}
    Sleep  5s
    Check Call Push  ${fourth_order}[plate_no]
    Check In Order  ${second_order}[order_number]
    Call Order  ${third_order}[order_number]  is_call=${False}
    Sleep  5s
    Check Call Push  ${third_order}[plate_no]
Auto Call When Finish
    [Teardown]  Del Exist Order
    Gate Close  ${s_fgate}
    Gate Close  ${s_bgate}
    @{multi_orders}  Make Multi Info
    Create Mult Order  @{multi_orders}
    Set Auto Call Count  1
    ${first_order}  Get From List  ${multi_orders}  0
    ${second_order}  Get From List  ${multi_orders}  1
    ${third_order}  Get From List  ${multi_orders}  2
    ${fourth_order}  Get From List  ${multi_orders}  3
    Check In Order  ${first_order}[order_number]
    Check In Order  ${second_order}[order_number]
    Check In Order  ${third_order}[order_number]
    Check In Order  ${fourth_order}[order_number]
    One Time Scale  p  23.1  trig_plate_no=${first_order}[plate_no]
    Confirm Order  ${first_order}[order_number]
    One Time Scale  m  29.1  trig_plate_no=${first_order}[plate_no]
    Check Call Push  ${second_order}[plate_no]

Auto Pass After Time Out
    [Teardown]  Del Exist Order
    Connect ZH
    Run ZH Cmd  rule
    Run ZH Cmd  call_time_out 1
    DisConnect
    @{multi_orders}  Make Multi Info
    Create Mult Order  @{multi_orders}
    Set Auto Call Count  1
    ${first_order}  Get From List  ${multi_orders}  0
    Check In Order  ${first_order}[order_number]
    Sleep  2s
    @{reg_orders}  Get Reg Orders
    Length Should Be  ${reg_orders}  length=1
    Sleep  2m
    @{reg_orders}  Get Reg Orders
    Length Should Be  ${reg_orders}  length=0

Auto Pass Before Time Out
    [Teardown]  Del Exist Order
    Connect ZH
    Run ZH Cmd  rule
    Run ZH Cmd  call_time_out 3
    DisConnect
    @{multi_orders}  Make Multi Info
    Create Mult Order  @{multi_orders}
    Set Auto Call Count  1
    ${first_order}  Get From List  ${multi_orders}  0
    Check In Order  ${first_order}[order_number]
    Sleep  2s
    @{reg_orders}  Get Reg Orders
    Length Should Be  ${reg_orders}  length=1
    Sleep  2m
    @{reg_orders}  Get Reg Orders
    Length Should Be  ${reg_orders}  length=1

No Auto Pass
    [Teardown]  Del Exist Order
    Connect ZH
    Run ZH Cmd  rule
    Run ZH Cmd  call_time_out 0
    DisConnect
    @{multi_orders}  Make Multi Info
    Create Mult Order  @{multi_orders}
    Set Auto Call Count  1
    ${first_order}  Get From List  ${multi_orders}  0
    Check In Order  ${first_order}[order_number]
    Sleep  2s
    @{reg_orders}  Get Reg Orders
    Length Should Be  ${reg_orders}  length=1
    Sleep  2m
    @{reg_orders}  Get Reg Orders
    Length Should Be  ${reg_orders}  length=1

*** Keywords ***
Make Multi Info
    @{m_orders}  Create List
    FOR  ${index}  IN RANGE  4
        &{one_order}  Create Dictionary  plate_no=${m_pl}[${index}]  driver_phone=${m_dh}[${index}]
        Append To List  ${m_orders}  ${one_order}
    END
    RETURN  @{m_orders}