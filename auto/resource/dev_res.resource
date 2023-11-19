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
    Run ZH Cmd  bdr
    Write  reboot
    DisConnect
    Sleep  5s
Clear Dev Config
    Connect ZH
    Run ZH Cmd  device
    Run ZH Cmd  clear_device
    DisConnect
Get Dev Status
    [Arguments]  ${dev_name}
    Connect SH
    Write  cat /tmp/${dev_name}.txt
    ${status}  Read Until Prompt  strip_prompt=True
    ${status}  Encode String To Bytes  ${status}  iso-8859-1
    ${status}  Decode Bytes To String  ${status}  UTF-8
    DisConnect
    ${line}  Split To Lines  ${status}
    RETURN  ${line}[0]
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
Set Scale Weight
    [Arguments]  ${weight}  ${dev_name}
    ${dev_id}  Get Dev ID by Name  dev_name=${dev_name}
    Connect ZH
    Run ZH Cmd  device
    Run ZH Cmd  mock_device_action ${dev_id} scale ${weight}
    DisConnect