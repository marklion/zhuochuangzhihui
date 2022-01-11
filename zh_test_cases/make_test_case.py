#!/usr/bin/python3

import os
import sys

def get_other_param(lines, param):
    ret = []
    for single_line in lines:
        param_name = single_line.split(':')[0]
        if param_name != param:
            ret.append('[{}] = "空"'.format(param_name))
    return ret

def get_other_else_param(lines, param):
    ret = []
    for single_line in lines:
        param_name = single_line.split(':')[0]
        if param_name != param:
            ret.append('[{}] <> "空"'.format(param_name))
    return ret

def get_related_name(lines):
    ret = []
    for single_line in lines:
        param_name = single_line.split(':')[0]
        all_names = single_line.split(':')[1]
        has_nag_value = []
        for single_value in all_names.split(','):
            if single_value.startswith("~"):
                has_nag_value.append('[{}] = "{}"'.format(param_name, single_value[1:]))
        then_cmd = " AND ".join(get_other_param(lines, param_name))
        else_cmd = " AND ".join(get_other_else_param(lines, param_name))

        if len(has_nag_value) != 0:
            ret.append('IF {} THEN {} ELSE {};'.format(" OR ".join(has_nag_value),  then_cmd, else_cmd))

    return ret


def output_case(file_name):
    f = open(file_name)
    lines = f.readlines()
    param_define = ""
    new_lines = []
    cons_line = []
    for single_line in lines:
        if ':' in single_line:
            single_line = single_line[:-1]
            single_line = single_line + ',空'
            new_lines.append(single_line)
        else:
            cons_line.append(single_line)
    param_define = "\n".join(new_lines) + '\n' + '\n'.join(get_related_name(new_lines)) + '\n' + ''.join(cons_line)
    fo = open("/tmp/tmp_case.txt", "w")
    fo.write(param_define)
    fo.close()
    case_out = os.popen("/root/pict/pict {}".format(file_name))
    case_out_string = case_out.readlines();
    case_file = open(".".join(file_name.split(".")[:-1]) + ".csv", "w")
    case_write_list = []
    case_index = 0
    for single_case in case_out_string:
        case_line = single_case[:-1].split("\t")
        if 0 == case_index:
            case_line.append("期望现象")
        elif "~" in single_case:
            for i in range(0, len(case_line)):
                if "~" not in case_line[i]:
                    case_line[i] = "空"
            case_line.append("禁止运行或无现象")
        else:
            case_line.append("运行成功")
        case_write_list.append(','.join(case_line))
        case_index += 1
    case_write_list = sorted(set(case_write_list), key=case_write_list.index)
    case_file.write('\n'.join(case_write_list));

def main_func(file_name):
    if file_name.split(".")[-1] == "txt":
        output_case(file_name)
    elif os.path.isdir(file_name):
        os.system("rm {}/*.csv".format(file_name))
        sub_file = os.listdir(file_name)
        for single_sub in  sub_file:
            main_func(file_name + '/' + single_sub)


if __name__ == "__main__":
    main_func(sys.argv[1])
