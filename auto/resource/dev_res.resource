*** Settings ***
Resource  cli_res.resource


*** Variables ***
${s_fpc}  fpc
${s_bpc}  bpc
${s_fgate}  fg
${s_bgate}  bg
${s_fled}  fl
${s_bled}  bl
${s_speaker}  fs
${s_fid}  fi
${s_bid}  bi
${s_s}  ss

*** Keywords ***

Clear Dev Config
    Connect ZH
    Run ZH Cmd  device
    Run ZH Cmd  clear_device
    DisConnect
Get Dev Status
    [Arguments]  ${dev_name}
    [Teardown]  DisConnect
    Connect SH
    Write  cat /tmp/${dev_name}.txt
    ${status}  Read Until Prompt  strip_prompt=True
    ${status}  Encode String To Bytes  ${status}  iso-8859-1
    ${status}  Decode Bytes To String  ${status}  UTF-8
    ${line}  Split To Lines  ${status}
    RETURN  ${line}[0]
Verify Gate Status
    [Arguments]  ${dev_name}  ${expect}=open
    ${status}  Get Dev Status  ${dev_name}
    IF  $expect == 'open'
        Should Be Equal As Strings  ${status}  开门
    ELSE
        Should Be Equal As Strings  ${status}  关门
    END
Verify LED Content
    [Arguments]  ${dev_name}  ${content}  ${line_num}=0
    [Teardown]  DisConnect
    Connect SH
    Write  cat /tmp/${dev_name}.txt
    ${status}  Read Until Prompt  strip_prompt=True
    ${status}  Encode String To Bytes  ${status}  iso-8859-1
    ${status}  Decode Bytes To String  ${status}  UTF-8
    ${line_focus}  Get Line  ${status}  ${line_num}
    Should Be Equal As Strings  ${line_focus}  ${content}
Verify Speaker Content
    [Arguments]  ${dev_name}  ${content}=${EMPTY}
    ${status}  Get Dev Status  ${dev_name}
    Should Be Equal As Strings  ${status}  ${content}

Get Dev ID by Name
    [Arguments]  ${dev_name}
    Connect ZH
    Run ZH Cmd  device
    ${all_dev_out}  Run ZH Cmd  show_device_scale
    ${lines}  Get Lines Containing String  ${all_dev_out}  ${dev_name}
    ${line}  Get Line  ${lines}  0
    ${line_array}  Split String  ${line}  :
    ${dev_name_array}  Split String From Right  ${line_array[0]}
    ${dev_id}  Set Variable  ${dev_name_array[-1]}
    DisConnect
    RETURN  ${dev_id}
Gate Close
    [Arguments]  ${dev_name}
    ${dev_id}  Get Dev ID by Name  dev_name=${dev_name}
    Connect ZH
    Run ZH Cmd  device
    Run ZH Cmd  mock_device_action ${dev_id} gate
    DisConnect
Mock Plate Cap
    [Arguments]  ${plate_no}  ${dev_name}
    ${dev_id}  Get Dev ID by Name  dev_name=${dev_name}
    Connect ZH
    Run ZH Cmd  device
    Run ZH Cmd  mock_device_action ${dev_id} plate ${plate_no}
    DisConnect
Mock Id Read
    [Arguments]  ${id_no}  ${dev_name}
    ${dev_id}  Get Dev ID by Name  dev_name=${dev_name}
    Connect ZH
    Run ZH Cmd  device
    Run ZH Cmd  mock_device_action ${dev_id} id_reader ${id_no}
    DisConnect
Set Scale Weight
    [Arguments]  ${weight}  ${dev_name}
    ${dev_id}  Get Dev ID by Name  dev_name=${dev_name}
    Connect ZH
    Run ZH Cmd  device
    Run ZH Cmd  mock_device_action ${dev_id} scale ${weight}
    DisConnect
Verify LED with Plate
    [Arguments]  ${plate_no}  ${content}  ${dev_name}
    Verify LED Content  ${dev_name}  ${content}  2
    IF  $plate_no != ''
        Verify LED Content  ${dev_name}  ${plate_no}  1
    END
One Time Scale
    [Arguments]  ${p_or_m}  ${weight}  ${cam_or_id}=cam  ${trig_plate_no}=  ${trig_driver_id}=
    ${e_gate}  Set Variable  ${s_fgate}
    ${l_gate}  Set Variable  ${s_bgate}
    ${pc}  Set Variable  ${s_fpc}
    ${id_reader}  Set Variable  ${s_fid}
    ${e_led}  Set Variable  ${s_fled}
    ${l_led}  Set Variable  ${s_bled}
    ${e_speaker}  Set Variable  ${s_speaker}
    ${l_speaker}  Set Variable  ${s_speaker}
    IF  $p_or_m == 'm'
        Set Test Variable  ${e_gate}  ${s_bgate}
        Set Test Variable  ${l_gate}  ${s_fgate}
        Set Test Variable  ${pc}  ${s_bpc}
        Set Test Variable  ${id_reader}  ${s_bid}
        Set Test Variable  ${e_led}  ${s_bled}
        Set Test Variable  ${l_led}  ${s_fled}
    END
    IF  $cam_or_id == 'cam'
        Mock Plate Cap  ${trig_plate_no}  ${pc}
    ELSE
        Mock Id Read  ${trig_driver_id}  ${id_reader}
    END
    Sleep  1s
    Verify LED with Plate  ${trig_plate_no}  请上磅  ${e_led}
    Verify LED with Plate  ${trig_plate_no}  请上磅  ${l_led}
    Sleep  3s
    Verify Gate Status  ${e_gate}  open
    Verify Gate Status  ${l_gate}  close
    Verify LED with Plate  ${trig_plate_no}  车辆未完全上磅  ${e_led}
    Verify LED with Plate  ${trig_plate_no}  车辆未完全上磅  ${l_led}
    Gate Close  ${e_gate}
    Set Scale Weight  ${weight}  ${s_s}
    Sleep  5s
    Verify LED with Plate  ${trig_plate_no}   请等待  ${e_led}
    Verify LED with Plate  ${trig_plate_no}   请等待  ${l_led}
    Sleep  17s
    Verify LED with Plate  ${trig_plate_no}   已完成，请下磅  ${e_led}
    Verify LED with Plate  ${trig_plate_no}   已完成，请下磅  ${l_led}
    Verify Gate Status  ${l_gate}  open
    Verify Gate Status  ${e_gate}  close
    Set Scale Weight  0  ${s_s}
    Gate Close  ${l_gate}
    Sleep  5s
Config Best Dev
    Connect ZH
    Run ZH Cmd  device
    Run ZH Cmd  add_set s 1
    Run ZH Cmd  add_device ${s_fpc} '-t s' 1 s front_plate_cam
    Run ZH Cmd  add_device ${s_bpc} '-t s' 1 s back_plate_cam
    Run ZH Cmd  add_device ${s_fled} '-t s' 1 s front_led
    Run ZH Cmd  add_device ${s_bled} '-t s' 1 s back_led
    Run ZH Cmd  add_device ${s_fid} '-t s' 1 s front_id_reader
    Run ZH Cmd  add_device ${s_bid} '-t s' 1 s back_id_reader
    Run ZH Cmd  add_device ${s_speaker} '-t s' 1 s back_speaker
    Run ZH Cmd  add_device ${s_fgate} '-t s' 1 s front_gate
    Run ZH Cmd  add_device ${s_bgate} '-t s' 1 s back_gate
    Run ZH Cmd  add_device ${s_speaker} '-t s' 1 s back_speaker
    Run ZH Cmd  add_device ${s_s} '-t s' 1 s scale
    Run ZH Cmd  zczh
    Run ZH Cmd  rule
    Run ZH Cmd  set_zyzl_plugin http://192.168.0.11:5001 abc
    Run ZH Cmd  set_ticket_prefix WR
    Run ZH Cmd  auto_call_count 2
    Run ZH Cmd  zczh
    Run ZH Cmd  bdr
    Write  reboot
    DisConnect
    Sleep  4s
    Gate Close  ${s_fgate}
    Gate Close  ${s_bgate}