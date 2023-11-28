*** Settings ***
Resource  ../resource/dev_res.resource
Resource  ../resource/rest_res.resource
Resource  ../test_field/field_opt.resource
Resource  running_opt.resource
Resource  ../test_order/order_opt.resource
Suite Setup  Config Best Dev
Suite Teardown  Clear Dev Config

*** Test Cases ***
Kill one of Dev Driver
    Connect SH
    ${old_result}  Execute Command  ps -ef | grep mock_ | grep -v grep | wc -l
    Execute Command  ps -ef | grep mock_ | grep -v grep | head -n 1 | awk '{print $2}' | xargs kill -9
    ${new_result}  Execute Command  ps -ef | grep mock_ | grep -v grep | grep -v '\\[mock_' | wc -l
    ${diff}  Evaluate  ${old_result} - ${new_result}
    Should Be Equal As Integers  ${diff}  1
    Sleep  20s
    ${new_result}  Execute Command  ps -ef | grep mock_ | grep -v grep | grep -v '\\[mock_' | wc -l
    Should Be Equal As Integers  ${new_result}  ${old_result}

Kill most of Dev Driver
    Connect SH
    ${old_result}  Execute Command  ps -ef | grep mock_ | grep -v grep | wc -l
    Execute Command  ps -ef | grep mock_ | grep -v grep | head -n 1 | awk '{print $2}' | xargs kill -9
    Execute Command  ps -ef | grep mock_ | grep -v grep | head -n 1 | awk '{print $2}' | xargs kill -9
    Execute Command  ps -ef | grep mock_ | grep -v grep | head -n 1 | awk '{print $2}' | xargs kill -9
    Execute Command  ps -ef | grep mock_ | grep -v grep | head -n 1 | awk '{print $2}' | xargs kill -9
    Sleep  21s
    ${new_result}  Execute Command  ps -ef | grep mock_ | grep -v grep | grep -v '\\[mock_' | wc -l
    Should Be Equal As Integers  ${new_result}  ${old_result}

Gate Ctrl Api Test
    Gate Close  ${s_bgate}
    Gate Close  ${s_fgate}
    Sleep  2s
    Gate Ctrl Api  ${s_bgate}  ${True}
    Sleep  2s
    Verify Gate Status  ${s_bgate}  open
    Gate Ctrl Api  ${s_fgate}  ${True}
    Sleep  2s
    Verify Gate Status  ${s_fgate}  open
    Gate Ctrl Api  ${s_bgate}  ${False}
    Sleep  2s
    Verify Gate Status  ${s_bgate}  close
    Gate Ctrl Api  ${s_fgate}  ${False}
    Sleep  2s
    Verify Gate Status  ${s_fgate}  close

Manual Cap Test
    [Setup]  Del Exist Order
    Plate Cap Api  ${s_fpc}
    Verify LED Content  ${s_fled}  无车辆信息  1

Take Pic Test
    ${pic_path}  Cap Picture Api  ${s_bpc}
    Log  ${pic_path}