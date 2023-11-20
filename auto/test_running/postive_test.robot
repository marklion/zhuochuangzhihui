*** Settings ***
Resource  ../resource/dev_res.resource
Resource  ../resource/rest_res.resource
Resource  ../test_field/field_opt.resource
Suite Setup  Config Best Dev
Suite Teardown  Clear Dev Config

*** Variables ***
${plate_no}  京A12345
${driver_phone}  18811111111
${driver_id}  123456789012345678
${company_name}  京东
${stuff_name}  LNG
${order_number}
${p_weight}  20.23
${m_weight}  49.21

*** Test Cases ***
Only Scale Full Flow
    [Setup]  Create A Order For Scale
    [Teardown]  Del Order Try  ${order_number}
    Check In Order  ${order_number}
    Call Order  ${order_number}
    One Time Scale  p
    Sleep  20s
    Confirm Order  order_number=${order_number}
    One Time Scale  m

Only Scale p_weight Twice
    [Setup]  Create A Order For Scale
    [Teardown]  Del Order Try  ${order_number}
    Check In Order  ${order_number}
    Call Order  ${order_number}
    One Time Scale  p
    Sleep  20s
    One Time Scale  m
    ${driver_result}  Search Order  driver_phone=${driver_phone}
    Should Not Be Empty  ${driver_result}
    Confirm Order  order_number=${order_number}
    One Time Scale  m


*** Keywords ***
Del Exist Order
    ${plate_result}  Search Order  ${plate_no}
    ${plate_result_length}  Get Length  ${plate_result}
    IF  ${plate_result_length} > 0
        Del Order Try  ${plate_result}[0][order_number]
    END
    ${driver_result}  Search Order  driver_phone=${driver_phone}
    ${driver_result_length}  Get Length  ${driver_result}
    IF  ${driver_result_length} > 0
        Del Order Try  ${plate_result}[0][order_number]
    END

Create A Order For Scale
    Del Exist Order
    Add Order Common  plate_no=${plate_no}  driver_phone=${driver_phone}  driver_id=${driver_id}  company_name=${company_name}  stuff_name=${stuff_name}
    ${driver_result}  Search Order  driver_phone=${driver_phone}
    Set Suite Variable  ${order_number}  ${driver_result}[0][order_number]
    Gate Close  ${s_fgate}
    Gate Close  ${s_bgate}

One Time Scale
    [Arguments]  ${p_or_m}
    ${weight}  Set Variable  ${p_weight}
    ${e_gate}  Set Variable  ${s_fgate}
    ${l_gate}  Set Variable  ${s_bgate}
    ${pc}  Set Variable  ${s_fpc}
    IF  $p_or_m == 'm'
        Set Test Variable  ${weight}  ${m_weight}
        Set Test Variable  ${e_gate}  ${s_bgate}
        Set Test Variable  ${l_gate}  ${s_fgate}
        Set Test Variable  ${pc}  ${s_bpc}
    END
    Mock Plate Cap  ${plate_no}  ${pc}
    Sleep  6s
    ${gate_status}  Get Dev Status  ${e_gate}
    Should Be Equal  ${gate_status}  开门
    ${gate_status}  Get Dev Status  ${l_gate}
    Should Be Equal  ${gate_status}  关门
    Gate Close  ${e_gate}
    Set Scale Weight  ${weight}  ${s_s}
    Sleep  18s
    ${gate_status}  Get Dev Status  ${l_gate}
    Should Be Equal  ${gate_status}  开门
    ${gate_status}  Get Dev Status  ${e_gate}
    Should Be Equal  ${gate_status}  关门
    Gate Close  ${l_gate}
    Set Scale Weight  0  ${s_s}

