*** Settings ***
Resource  ../resource/dev_res.resource
Resource  ../resource/rest_res.resource
Library  Collections
Library  DateTime

*** Keywords ***
Del Order
    [Arguments]  ${order_id}
    ${del_req}  Create Dictionary  order_number=${order_id}
    POST to Server Success  /order/del  ${del_req}
Del Exist Order
    ${plate_result}  Search Order
    ${plate_result_length}  Get Length  ${plate_result}
    FOR  ${order}  IN  @{plate_result}
        Del Order  ${order}[order_number]
    END
Search Order
    [Arguments]  ${plate_no}=  ${driver_phone}=
    ${search_condition}  Create Dictionary  plate_number=${plate_no}  driver_phone=${driver_phone}  exp_status=${100}
    ${search_result}  POST to Server Success  /order/search  ${search_condition}
    RETURN  ${search_result}
Add Order Common
    [Arguments]  ${plate_no}  ${driver_phone}  ${company_name}=测试公司  ${stuff_name}=测试物料  ${driver_id}=  ${driver_name}=
    ${order_data}  Create Dictionary  plate_number=${plate_no}  driver_phone=${driver_phone}  company_name=${company_name}  stuff_name=${stuff_name}  driver_id=${driver_id}  driver_name=${driver_name}
    POST to Server Success  /order/add  ${order_data}
    ${driver_result}  Search Order  driver_phone=${driver_phone}
    RETURN  ${driver_result}[0][order_number]
Get Order By Order Number Not Exist
    [Arguments]  ${order_number}
    ${search_condition}  Create Dictionary  order_number=${order_number}
    POST to Server Failure  /order/get  ${search_condition}
Get Order By Order Number Exist
    [Arguments]  ${order_number}
    ${search_condition}  Create Dictionary  order_number=${order_number}
    ${search_result}  POST to Server Success  /order/get  ${search_condition}
    RETURN  ${search_result}
Create Mult Order
    [Arguments]  @{m_order_info}
    Del Exist Order
    ${count}  Get Length  ${m_order_info}
    FOR  ${index}  IN RANGE  ${count}
        ${on}  Add Order Common  ${m_order_info}[${index}][plate_no]  ${m_order_info}[${index}][driver_phone]
        Set To Dictionary  ${m_order_info}[${index}]  order_number  ${on}
    END
Check Order Status
    [Arguments]  ${order_number}  ${open_or_close}=open
    ${found_order}  Get Order By Order Number Exist  order_number=${order_number}
    IF  $open_or_close == 'open'
        Should Not Be Equal As Integers  ${found_order}[status]  100
    ELSE
        Should Be Equal As Integers  ${found_order}[status]  100
    END