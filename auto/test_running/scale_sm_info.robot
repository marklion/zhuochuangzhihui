*** Settings ***
Resource  running_opt.resource
Resource  ../resource/dev_res.resource
Resource  ../test_order/order_opt.resource
Resource  ../test_field/field_opt.resource
Suite Setup  Run Keywords  Config Best Dev  AND  Start Mock Service
Suite Teardown  Run Keywords  Clear Dev Config  AND  Stop Mock Service

*** Variables ***
${plate}  京A12345
${phone}  11122222211

*** Test Cases ***
Init State of Scale
    Check Default Sm State  空闲  0
    Set Scale Weight  18.3  ${s_s}
    Sleep  4s
    Check Default Sm State  空闲  18.3
    Set Scale Weight  0  ${s_s}
    Sleep  4s
    Check Default Sm State  空闲  0
Prep State of Scale
    [Teardown]  Run Keywords  Del Exist Order  AND  Gate Close  ${s_fgate}  AND  Gate Close  ${s_bgate}
    ${one_scale}  Get First Scale Sm Info
    Set Scale Weight  1.3  ${s_s}
    Sleep  4s
    ${on}  Add Order Common  ${plate}  ${phone}
    Mock Plate Cap  plate_no=${plate}  dev_name=${s_fpc}
    Check Default Sm State  空闲  1.3
    Check In Order  ${on}
    Mock Plate Cap  plate_no=${plate}  dev_name=${s_fpc}
    Check Default Sm State  准备  1.3  ${plate}
    Set Scale Weight  18.3  ${s_s}
    Sleep  4s
    Check Default Sm State  准备  18.3  ${plate}
    Set Scale Weight  0  ${s_s}
    Sleep  4s
    Reset Scale Sm By Api  ${one_scale}[set_info][id]
    Check Default Sm State  空闲  0
Prep State meet Gate Open
    [Teardown]  Run Keywords  Del Exist Order  AND  Gate Close  ${s_fgate}  AND  Gate Close  ${s_bgate}
    ${one_scale}  Get First Scale Sm Info
    ${on}  Add Order Common  ${plate}  ${phone}
    Check In Order  ${on}
    Mock Plate Cap  plate_no=${plate}  dev_name=${s_fpc}
    Set Scale Weight  20.3  ${s_s}
    Sleep  13s
    Check Default Sm State  准备  20.3  ${plate}
    Confirm Scale Sm  ${one_scale}[set_info][id]
    Sleep  1s
    Check Default Sm State  称重  20.3  ${plate}
    Sleep  16s
    Set Scale Weight  0  ${s_s}
    Sleep  4s
    Gate Close  ${s_fgate}
    Gate Close  ${s_bgate}
    Confirm Order  ${on}
    One Time Scale  p  49.9  trig_plate_no=${plate}


Scale State of Scale
    [Teardown]  Run Keywords  Del Exist Order  AND  Gate Close  ${s_fgate}  AND  Gate Close  ${s_bgate}
    ${one_scale}  Get First Scale Sm Info
    ${on}  Add Order Common  ${plate}  ${phone}
    Check In Order  ${on}
    Mock Plate Cap  plate_no=${plate}  dev_name=${s_fpc}
    Sleep  3s
    Gate Close  ${s_fgate}
    Sleep  3s
    @{scale_list}  Create List  23.1  25.6  33.4  44.23  32.1  54.1  23.6  49.1
    FOR  ${weight}  IN  @{scale_list}
        Set Scale Weight  ${weight}  ${s_s}
        Sleep  4s
        Check Default Sm State  称重   ${weight}  plate=${plate}
    END
    Set Scale Weight  0  ${s_s}
    Sleep  4s
    Reset Scale Sm By Api  ${one_scale}[set_info][id]
    Check Default Sm State  空闲  0
    ${now_on}  Get Order By Order Number Exist  ${on}
    Should Be Equal As Integers  ${now_on}[p_weight]  0
Scale State Meet No Confirm
    [Teardown]  Run Keywords  Del Exist Order  AND  Gate Close  ${s_fgate}  AND  Gate Close  ${s_bgate}
    ${one_scale}  Get First Scale Sm Info
    ${on}  Add Order Common  ${plate}  ${phone}
    Check In Order  ${on}
    One Time Scale  p  18.9  trig_plate_no=${plate}
    Mock Plate Cap  plate_no=${plate}  dev_name=${s_fpc}
    Sleep  3s
    Gate Close  ${s_fgate}
    Sleep  3s
    Set Scale Weight  34.01  ${s_s}
    Sleep  21s
    Check Default Sm State  称重   34.01  plate=${plate}
    Verify LED with Plate  ${plate}  未确认装卸货  ${s_bled}
    Verify LED with Plate  ${plate}  未确认装卸货  ${s_fled}
    Reset Scale Sm By Cli  scale_sm_name=${one_scale}[set_info][name]
    Check Default Sm State  空闲  34.01
    Mock Plate Cap  plate_no=${plate}  dev_name=${s_fpc}
    Sleep  3s
    Gate Close  ${s_fgate}
    Sleep  3s
    Set Scale Weight  34.01  ${s_s}
    Sleep  21s
    Check Default Sm State  称重   34.01  plate=${plate}
    Confirm Order  ${on}
    Sleep  4s
    Check Default Sm State  清理  34.01  plate=${plate}
    Set Scale Weight  0  ${s_s}
    Sleep  4s
    Check Default Sm State  空闲  0

Clean State of Scale
    [Teardown]  Run Keywords  Del Exist Order  AND  Gate Close  ${s_fgate}  AND  Gate Close  ${s_bgate}
    ${one_scale}  Get First Scale Sm Info
    ${on}  Add Order Common  ${plate}  ${phone}
    Check In Order  ${on}
    Confirm Order  ${on}
    Mock Plate Cap  plate_no=${plate}  dev_name=${s_fpc}
    Sleep  3s
    Gate Close  ${s_fgate}
    Set Scale Weight  18.3  ${s_s}
    Sleep  21s
    Check Default Sm State  清理  18.3  ${plate}
    Gate Close  ${s_bgate}
    Sleep  13s
    Check Default Sm State  清理  18.3  ${plate}
    Reset Scale Sm By Cli  ${one_scale}[set_info][name]
    Check Default Sm State  空闲  18.3
    Set Scale Weight  0  ${s_s}
    Sleep  4s
    One Time Scale  m  34.1  trig_plate_no=${plate}
    Check Default Sm State  空闲  0


*** Keywords ***
Check Default Sm State
    [Arguments]  ${state}  ${weight}  ${plate}=
    ${sm}  Get First Scale Sm Info
    Should Be Equal As Strings  ${sm}[cur_state]  ${state}
    Should Be Equal As Numbers  ${sm}[cur_weight]  ${weight}
    Should Be Equal As Strings  ${sm}[cur_plate]  ${plate}

