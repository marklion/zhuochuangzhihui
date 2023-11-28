*** Settings ***
Resource  ../resource/dev_res.resource
Resource  ../resource/rest_res.resource
Resource  ../test_field/field_opt.resource
Resource  ../test_order/order_opt.resource
Suite Setup  Run Keywords  Config Best Dev  AND  Start Mock Service
Suite Teardown  Run Keywords  Clear Dev Config  AND  Stop Mock Service
*** Variables ***
${plate_no}  京A12345
${driver_phone}  18811111111
${driver_id}  123456789012345678
${company_name}  京东
${stuff_name}  LNG
${order_number}
${p_weight}  20.23
${m_weight}  49.21
${driver_name}  张三

@{m_pl}  京A12345  京A12346  京A12347  京A12348
@{m_dh}  18811111111  18811111112  18811111113  18811111114

*** Test Cases ***
Multi Time Scale Full Flow
    [Teardown]  Del Exist Order
    @{m_orders}  Make Multi Info
    Create Mult Order  @{m_orders}
    ${last_ticket_no}  Set Variable  0
    FOR  ${itr_order}  IN  @{m_orders}
        Check In Order  ${itr_order}[order_number]
        Confirm Order  ${itr_order}[order_number]
        One Time Scale  p  ${p_weight}  trig_plate_no=${itr_order}[plate_no]
        One Time Scale  m  ${m_weight}  trig_plate_no=${itr_order}[plate_no]
        Check Order Status  ${itr_order}[order_number]  close
        ${this_ticket_no}  Get Ticket Last 4 No
        IF  ${last_ticket_no} != 0
            ${inc_count}  Evaluate  ${this_ticket_no} - ${last_ticket_no}
            Should Be Equal As Integers  ${inc_count}  1
        END
        ${last_ticket_no}  Set Variable  ${this_ticket_no}
    END

Only Scale Full Flow
    [Setup]  Create A Order For Scale
    [Teardown]  Del Order  ${order_number}
    Check In Order  ${order_number}
    Sleep  6s
    Check Call Push  ${plate_no}  driver_name=${driver_name}
    One Time Scale  p  ${p_weight}  cam_or_id=id  trig_driver_id=${driver_id}
    Check Order Status  ${order_number}
    Confirm Order  ${order_number}
    One Time Scale  m  ${m_weight}  trig_plate_no=${plate_no}
    Check Order Status  ${order_number}  close
    Check Weight Push  ${plate_no}  ${p_weight}  ${m_weight}
*** Keywords ***
Create A Order For Scale
    Del Exist Order
    ${on}  Add Order Common  plate_no=${plate_no}  driver_phone=${driver_phone}  driver_id=${driver_id}  company_name=${company_name}  stuff_name=${stuff_name}  driver_name=${driver_name}
    Set Suite Variable  ${order_number}  ${on}
    Gate Close  ${s_fgate}
    Gate Close  ${s_bgate}


Make Multi Info
    @{m_orders}  Create List
    FOR  ${index}  IN RANGE  4
        ${one_order}  Create Dictionary  plate_no=${m_pl}[${index}]  driver_phone=${m_dh}[${index}]  company_name=${company_name}  stuff_name=${stuff_name}
        Append To List  ${m_orders}  ${one_order}
    END

    RETURN  @{m_orders}
