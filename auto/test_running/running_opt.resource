*** Settings ***
Resource  ../resource/rest_res.resource
Resource  ../resource/dev_res.resource

*** Keywords ***
Get Scale Sm Info
    ${resp}  POST to Server Success  /get_scale_sm_info
    RETURN  ${resp}
Get First Scale Sm Info
    ${resp}  Get Scale Sm Info
    Should Be Equal As Strings  ${resp}[0][set_info][name]  s
    RETURN  ${resp}[0]
Reset Scale Sm By Api
    [Arguments]  ${scale_sm_id}
    &{req}  Create Dictionary  sm_id=${scale_sm_id}
    ${resp}  POST to Server Success  /reset_scale_sm  ${req}
Reset Scale Sm By Cli
    [Arguments]  ${scale_sm_name}
    [Teardown]  DisConnect
    Connect ZH
    Run ZH Cmd  device
    Run ZH Cmd  reset_scale ${scale_sm_name}
Confirm Scale Sm
    [Arguments]  ${scale_sm_id}
    &{req}  Create Dictionary  sm_id=${scale_sm_id}
    ${resp}  POST to Server Success  /confirm_scale  ${req}
Gate Ctrl Api
    [Arguments]  ${device_name}  ${is_open}={True}
    ${device_id}  Get Dev ID by Name  ${device_name}
    ${device_id}=  Convert To Integer  ${device_id}
    &{req}  Create Dictionary  device_id=${device_id}  is_open=${is_open}
    POST to Server Success  /gate_ctrl  ${req}
Plate Cap Api
    [Arguments]  ${device_name}
    ${device_id}  Get Dev ID by Name  ${device_name}
    ${device_id}=  Convert To Integer  ${device_id}
    &{req}  Create Dictionary  device_id=${device_id}
    POST to Server Success  /plate_cam_cap  ${req}
Cap Picture Api
    [Arguments]  ${device_name}
    ${device_id}  Get Dev ID by Name  ${device_name}
    ${device_id}=  Convert To Integer  ${device_id}
    &{req}  Create Dictionary  device_id=${device_id}
    ${result}  POST to Server Success  /cap_picture  ${req}
    RETURN  ${result}




