*** Settings ***
Resource  ../resource/dev_res.resource
Resource  ../resource/rest_res.resource
Resource  ../test_field/field_opt.resource
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