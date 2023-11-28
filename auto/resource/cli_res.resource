*** Settings ***
Library    SSHLibrary
Library  String

*** Variables ***
${super_user_name}  su
${super_user_phone}  111
${super_user_pwd}  698D51A19D8A121CE581499D7B701668

*** Keywords ***
Run ZH Cmd
    [Arguments]  ${cmd}
    Log  ${cmd}
    Write  ${cmd}
    ${output}  Read Until Prompt  loglevel=INFO  strip_prompt=True
    Log  ${output}
    ${output}  Encode String To Bytes  ${output}  iso-8859-1
    ${output}  Decode Bytes To String  ${output}  UTF-8
    RETURN  ${output}
Connect SH
    Open Connection    localhost    prompt=REGEXP:.*[>#]$  port=8222  escape_ansi=True  term_type=vt100  timeout=9s
    Login   root    P@ssw0rd
    Write  source ~/.bashrc
    Read Until Prompt
Connect ZH
    Connect SH
    Run ZH Cmd  zh_cli -d
DisConnect
    Close Connection
Create Max Pri User
    [Teardown]  DisConnect
    Connect ZH
    Run ZH Cmd  rbac
    Run ZH Cmd  add_user ${super_user_name} ${super_user_phone} 111
    ${user_list}  Run ZH Cmd  show_users
    ${lines}  Get Lines Containing String  ${user_list}  ${super_user_name}
    ${line}  Get Line  ${lines}  0
    ${line_array}  Split String  ${line}  |
    ${user_id}  Set Variable  ${line_array[1]}
    Run ZH Cmd  add_user_to_role 1 ${user_id}
Get Max Pri User
    RETURN  ${super_user_phone}  ${super_user_pwd}

