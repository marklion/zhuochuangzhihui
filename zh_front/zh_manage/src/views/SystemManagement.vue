<template>
<div class="system_management_show">
    <el-tabs v-model="activeName">
        <el-tab-pane label="设备配置" name="device_config">
            <div class="device_config_show" v-for="(single_gate, index) in device_config.gate" :key="'gate' + index">
                <el-descriptions :column="4" border :title="single_gate.name">
                    <el-descriptions-item label="入口抓拍机IP">{{single_gate.entry_config.cam_ip}}</el-descriptions-item>
                    <el-descriptions-item label="入口LEDIP">{{single_gate.entry_config.led_ip}}</el-descriptions-item>
                    <el-descriptions-item label="入口 NVR IP">{{single_gate.entry_nvr_ip}}</el-descriptions-item>
                    <el-descriptions-item label="入口 通道">{{single_gate.entry_channel}}</el-descriptions-item>
                    <el-descriptions-item label="入口 NVR 用户名">{{single_gate.entry_login.username}}</el-descriptions-item>
                    <el-descriptions-item label="入口 NVR 密码">{{single_gate.entry_login.password}}</el-descriptions-item>
                    <el-descriptions-item label="出口抓拍机IP">{{single_gate.exit_config.cam_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口LEDIP">{{single_gate.exit_config.led_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口 NVR IP">{{single_gate.exit_nvr_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口 通道">{{single_gate.exit_channel}}</el-descriptions-item>
                    <el-descriptions-item label="出口 NVR 用户名">{{single_gate.exit_login.username}}</el-descriptions-item>
                    <el-descriptions-item label="出口 NVR 密码">{{single_gate.exit_login.password}}</el-descriptions-item>
                    <el-descriptions-item>
                        {{single_gate.entry_qr_ip}}
                        <template slot="label">
                            <span>入口二维码IP</span>
                            <span>
                                <el-tag v-if="single_gate.entry_need_qr" type="success">必选验证</el-tag>
                                <el-tag v-else>可选验证</el-tag>
                            </span>
                        </template>
                    </el-descriptions-item>
                    <el-descriptions-item>
                        {{single_gate.exit_qr_ip}}
                        <template slot="label">
                            <span>出口二维码IP</span>
                            <span>
                                <el-tag v-if="single_gate.exit_need_qr" type="success">必选验证</el-tag>
                                <el-tag v-else>可选验证</el-tag>
                            </span>
                        </template>

                    </el-descriptions-item>
                    <el-descriptions-item>
                        {{single_gate.entry_id_reader_ip}}
                        <template slot="label">
                            <span>入口身份识别IP</span>
                            <span>
                                <el-tag v-if="single_gate.entry_need_id" type="success">必选验证</el-tag>
                                <el-tag v-else>可选验证</el-tag>
                            </span>
                        </template>
                    </el-descriptions-item>
                    <el-descriptions-item>
                        <template slot="label">
                            <span>出口身份识别IP</span>
                            <span>
                                <el-tag v-if="single_gate.exit_need_id" type="success">必选验证</el-tag>
                                <el-tag v-else>可选验证</el-tag>
                            </span>
                        </template>
                        {{single_gate.exit_id_reader_ip}}
                    </el-descriptions-item>

                    <template slot="extra">
                        <el-button type="success" size="small" @click="open_gate_operate(single_gate)">操作</el-button>
                        <el-button type="primary" size="small" @click="open_gate_edit(single_gate)">编辑</el-button>
                    </template>
                </el-descriptions>
            </div>
            <div class="device_config_show" v-for="(single_scale, index) in device_config.scale" :key="'scale' + index">
                <el-descriptions border :column="4" :title="single_scale.name">
                    <el-descriptions-item label="入口抓拍机IP">{{single_scale.entry_config.cam_ip}}</el-descriptions-item>
                    <el-descriptions-item label="入口 NVR IP">{{single_scale.entry_nvr_ip}}</el-descriptions-item>
                    <el-descriptions-item label="入口 通道">{{single_scale.entry_channel}}</el-descriptions-item>
                    <el-descriptions-item label="入口 NVR 用户名">{{single_scale.entry_login.username}}</el-descriptions-item>
                    <el-descriptions-item label="入口 NVR 密码">{{single_scale.entry_login.password}}</el-descriptions-item>
                    <el-descriptions-item label="入口LEDIP">{{single_scale.entry_config.led_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口抓拍机IP">{{single_scale.exit_config.cam_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口 NVR IP">{{single_scale.exit_nvr_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口 通道">{{single_scale.exit_channel}}</el-descriptions-item>
                    <el-descriptions-item label="出口 NVR 用户名">{{single_scale.exit_login.username}}</el-descriptions-item>
                    <el-descriptions-item label="出口 NVR 密码">{{single_scale.exit_login.password}}</el-descriptions-item>
                    <el-descriptions-item label="出口LEDIP">{{single_scale.exit_config.led_ip}}</el-descriptions-item>
                    <el-descriptions-item label="入口打印机IP">{{single_scale.entry_printer_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口打印机IP">{{single_scale.exit_printer_ip}}</el-descriptions-item>
                    <el-descriptions-item label="入口身份识别IP">{{single_scale.entry_id_reader_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口身份识别IP">{{single_scale.exit_id_reader_ip}}</el-descriptions-item>
                    <el-descriptions-item label="入口二维码IP">{{single_scale.entry_qr_ip}}</el-descriptions-item>
                    <el-descriptions-item label="出口二维码IP">{{single_scale.exit_qr_ip}}</el-descriptions-item>
                    <el-descriptions-item label="衡器IP">{{single_scale.scale_ip}}</el-descriptions-item>
                    <el-descriptions-item label="衡器品牌">{{single_scale.scale_brand}}</el-descriptions-item>
                    <el-descriptions-item label="读数系数">{{single_scale.coefficient}}</el-descriptions-item>
                    <el-descriptions-item label="最低重量">{{single_scale.min_weight}}</el-descriptions-item>
                    <el-descriptions-item label="光栅IP-1">{{single_scale.raster_ip[0]}}</el-descriptions-item>
                    <el-descriptions-item label="光栅IP-2">{{single_scale.raster_ip[1]}}</el-descriptions-item>
                    <el-descriptions-item label="磅体1 NVR 用户名">{{single_scale.scale1.username}}</el-descriptions-item>
                    <el-descriptions-item label="磅体1 NVR 密码">{{single_scale.scale1.password}}</el-descriptions-item>
                    <el-descriptions-item label="磅体1 NVR IP">{{single_scale.scale1_nvr_ip}}</el-descriptions-item>
                    <el-descriptions-item label="磅体1 通道">{{single_scale.scale1_channel}}</el-descriptions-item>
                    <el-descriptions-item label="磅体2 NVR 用户名">{{single_scale.scale2.username}}</el-descriptions-item>
                    <el-descriptions-item label="磅体2 NVR 密码">{{single_scale.scale2.password}}</el-descriptions-item>
                    <el-descriptions-item label="磅体2 NVR IP">{{single_scale.scale2_nvr_ip}}</el-descriptions-item>
                    <el-descriptions-item label="磅体2 通道">{{single_scale.scale2_channel}}</el-descriptions-item>
                    <el-descriptions-item label="磅体3 NVR 用户名">{{single_scale.scale3.username}}</el-descriptions-item>
                    <el-descriptions-item label="磅体3 NVR 密码">{{single_scale.scale3.password}}</el-descriptions-item>
                    <el-descriptions-item label="磅体3 NVR IP">{{single_scale.scale3_nvr_ip}}</el-descriptions-item>
                    <el-descriptions-item label="磅体3 通道">{{single_scale.scale3_channel}}</el-descriptions-item>
                    <el-descriptions-item label="身份证验证">
                        <el-tag v-if="single_scale.need_id" type="success">必选验证</el-tag>
                        <el-tag v-else>可选验证</el-tag>
                    </el-descriptions-item>
                    <el-descriptions-item label="二维码验证">
                        <el-tag v-if="single_scale.need_qr" type="success">必选验证</el-tag>
                        <el-tag v-else>可选验证</el-tag>
                    </el-descriptions-item>
                    <template slot="extra">
                        <el-button type="success" size="small" @click="open_scale_operate(single_scale)">操作</el-button>
                        <el-button type="primary" size="small" @click="open_scale_edit(single_scale)">编辑</el-button>
                    </template>
                </el-descriptions>
            </div>
            <el-dialog @close="init_device_info" title="修改配置" :visible.sync="show_edit_gate_config" width="60%" @keyup.enter.native="edit_gate">
                <el-form :model="gate_for_edit" ref="edit_gate_form" :rules="rules" label-width="150px">
                    <el-form-item label="名称" prop="name">
                        <el-input v-model="gate_for_edit.name" placeholder="请输入名称"></el-input>
                    </el-form-item>
                    <el-form-item label="入口抓拍机IP" prop="entry_cam_ip">
                        <el-input v-model="gate_for_edit.entry_config.cam_ip" placeholder="请输入入口抓拍机IP"></el-input>
                    </el-form-item>
                    <el-form-item label="入口NVRIP" prop="entry_nvr_ip">
                        <el-input v-model="gate_for_edit.entry_nvr_ip" placeholder="请输入入口NVRIP"></el-input>
                    </el-form-item>
                    <el-form-item label="入口通道" prop="entry_channel">
                        <el-input v-model="gate_for_edit.entry_channel" type="number" placeholder="请输入入口通道"></el-input>
                    </el-form-item>
                    <el-form-item label="入口NVR用户名" prop="entry_nvr_username">
                        <el-input v-model="gate_for_edit.entry_login.username" placeholder="请输入入口NVR用户名"></el-input>
                    </el-form-item>
                    <el-form-item label="入口NVR密码" prop="entry_nvr_password">
                        <el-input v-model="gate_for_edit.entry_login.password" placeholder="请输入入口NVR密码"></el-input>
                    </el-form-item>
                    <el-form-item label="出口抓拍机IP" prop="exit_cam_ip">
                        <el-input v-model="gate_for_edit.exit_config.cam_ip" placeholder="请输入出口抓拍机IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口NVRIP" prop="entry_nvr_ip">
                        <el-input v-model="gate_for_edit.exit_nvr_ip" placeholder="请输入出口NVRIP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口通道" prop="exit_channel">
                        <el-input v-model="gate_for_edit.exit_channel" type="number" placeholder="请输入出口通道"></el-input>
                    </el-form-item>
                    <el-form-item label="入口LEDIP" prop="entry_led_ip">
                        <el-input v-model="gate_for_edit.entry_config.led_ip" placeholder="请输入入口LEDIP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口LEDIP" prop="exit_led_ip">
                        <el-input v-model="gate_for_edit.exit_config.led_ip" placeholder="请输入出口LEDIP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口NVR用户名" prop="exit_nvr_username">
                        <el-input v-model="gate_for_edit.exit_login.username" placeholder="请输入出口NVR用户名"></el-input>
                    </el-form-item>
                    <el-form-item label="出口NVR密码" prop="exit_nvr_password">
                        <el-input v-model="gate_for_edit.exit_login.password" placeholder="请输入出口NVR密码"></el-input>
                    </el-form-item>
                    <el-form-item label="入口身份识别IP" prop="entry_id_reader_ip">
                        <el-input v-model="gate_for_edit.entry_id_reader_ip" placeholder="请输入入口身份识别IP"></el-input>
                    </el-form-item>
                    <el-form-item label="入口必选身份证验证" prop="entry_need_id">
                        <el-switch v-model="gate_for_edit.entry_need_id"></el-switch>
                    </el-form-item>
                    <el-form-item label="出口身份识别IP" prop="exit_id_reader_ip">
                        <el-input v-model="gate_for_edit.exit_id_reader_ip" placeholder="请输入出口身份识别IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口必选身份证验证" prop="exit_need_id">
                        <el-switch v-model="gate_for_edit.exit_need_id"></el-switch>
                    </el-form-item>
                    <el-form-item label="入口二维码IP" prop="entry_qr_ip">
                        <el-input v-model="gate_for_edit.entry_qr_ip" placeholder="请输入入口二维码IP"></el-input>
                    </el-form-item>
                    <el-form-item label="入口必选二维码验证" prop="entry_need_qr">
                        <el-switch v-model="gate_for_edit.entry_need_qr"></el-switch>
                    </el-form-item>
                    <el-form-item label="出口二维码IP" prop="exit_qr_ip">
                        <el-input v-model="gate_for_edit.exit_qr_ip" placeholder="请输出入口二维码IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口必选二维码验证" prop="exit_need_qr">
                        <el-switch v-model="gate_for_edit.exit_need_qr"></el-switch>
                    </el-form-item>
                    <el-form-item>
                        <el-button type="primary" @click="edit_gate">确认</el-button>
                    </el-form-item>
                </el-form>
            </el-dialog>
            <el-dialog @close="init_device_info" title="修改配置" :visible.sync="show_edit_scale_config" width="60%" @keyup.enter.native="edit_scale">
                <el-form :model="scale_for_edit" ref="edit_scale_form" :rules="rules" label-width="120px">
                    <el-form-item label="名称" prop="name">
                        <el-input v-model="scale_for_edit.name" placeholder="请输入名称"></el-input>
                    </el-form-item>
                    <el-form-item label="入口抓拍机IP" prop="entry_cam_ip">
                        <el-input v-model="scale_for_edit.entry_config.cam_ip" placeholder="请输入入口抓拍机IP"></el-input>
                    </el-form-item>
                    <el-form-item label="入口NVRIP" prop="entry_nvr_ip">
                        <el-input v-model="scale_for_edit.entry_nvr_ip" placeholder="请输入入口NVRIP"></el-input>
                    </el-form-item>
                    <el-form-item label="入口通道" prop="entry_channel">
                        <el-input v-model="scale_for_edit.entry_channel" placeholder="请输入入口通道"></el-input>
                    </el-form-item>
                    <el-form-item label="入口NVR用户名" prop="entry_nvr_login_username">
                        <el-input v-model="scale_for_edit.entry_login.username"></el-input>
                    </el-form-item>
                    <el-form-item label="入口NVR密码" prop="entry_nvr_login_password">
                        <el-input v-model="scale_for_edit.entry_login.password"></el-input>
                    </el-form-item>
                    <el-form-item label="出口抓拍机IP" prop="exit_cam_ip">
                        <el-input v-model="scale_for_edit.exit_config.cam_ip" placeholder="请输入出口抓拍机IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口NVRIP" prop="entry_nvr_ip">
                        <el-input v-model="scale_for_edit.exit_nvr_ip" placeholder="请输入出口NVRIP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口通道" prop="exit_channel">
                        <el-input v-model="scale_for_edit.exit_channel" placeholder="请输入出口通道"></el-input>
                    </el-form-item>
                    <el-form-item label="出口NVR用户名" prop="exit_nvr_login_username">
                        <el-input v-model="scale_for_edit.exit_login.username"></el-input>
                    </el-form-item>
                    <el-form-item label="出口NVR密码" prop="exit_nvr_login_password">
                        <el-input v-model="scale_for_edit.exit_login.password"></el-input>
                    </el-form-item>
                    <el-form-item label="入口LEDIP" prop="entry_led_ip">
                        <el-input v-model="scale_for_edit.entry_config.led_ip" placeholder="请输入入口LEDIP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口LEDIP" prop="exit_led_ip">
                        <el-input v-model="scale_for_edit.exit_config.led_ip" placeholder="请输入出口LEDIP"></el-input>
                    </el-form-item>
                    <el-form-item label="衡器IP" prop="scale_ip">
                        <el-input v-model="scale_for_edit.scale_ip" placeholder="请输入衡器IP"></el-input>
                    </el-form-item>
                    <el-form-item label="衡器品牌" prop="scale_brand">
                        <item-for-select v-model="scale_for_edit.scale_brand" search_key="scale_brand"></item-for-select>
                    </el-form-item>
                    <el-form-item label="衡器读数系数" prop="coefficient">
                        <el-input v-model="scale_for_edit.coefficient" type="number" placeholder="请输入衡器系数"></el-input>
                    </el-form-item>
                    <el-form-item label="最低重量" prop="min_weight">
                        <el-input v-model="scale_for_edit.min_weight" type="number" placeholder="请输入最低重量"></el-input>
                    </el-form-item>
                    <el-form-item label="光栅IP-1" prop="raster_ip">
                        <el-input v-model="scale_for_edit.raster_ip[0]" placeholder="请输入光栅IP-1"></el-input>
                    </el-form-item>
                    <el-form-item label="光栅IP-2" prop="raster_ip">
                        <el-input v-model="scale_for_edit.raster_ip[1]" placeholder="请输入光栅IP-2"></el-input>
                    </el-form-item>
                    <el-form-item label="入口打印机IP" prop="entry_printer_ip">
                        <el-input v-model="scale_for_edit.entry_printer_ip" placeholder="请输入入口打印机IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口打印机IP" prop="exit_printer_ip">
                        <el-input v-model="scale_for_edit.exit_printer_ip" placeholder="请输入出口打印机IP"></el-input>
                    </el-form-item>
                    <el-form-item label="入口身份识别IP" prop="entry_id_reader_ip">
                        <el-input v-model="scale_for_edit.entry_id_reader_ip" placeholder="请输入入口身份识别IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口身份识别IP" prop="exit_id_reader_ip">
                        <el-input v-model="scale_for_edit.exit_id_reader_ip" placeholder="请输入出口身份识别IP"></el-input>
                    </el-form-item>
                    <el-form-item label="入口二维码IP" prop="entry_qr_ip">
                        <el-input v-model="scale_for_edit.entry_qr_ip" placeholder="请输入入口二维码IP"></el-input>
                    </el-form-item>
                    <el-form-item label="出口二维码IP" prop="exit_qr_ip">
                        <el-input v-model="scale_for_edit.exit_qr_ip" placeholder="请输出入口二维码IP"></el-input>
                    </el-form-item>
                    <el-form-item label="必选身份证验证" prop="need_id">
                        <el-switch v-model="scale_for_edit.need_id"></el-switch>
                    </el-form-item>
                    <el-form-item label="必选二维码验证" prop="need_qr">
                        <el-switch v-model="scale_for_edit.need_qr"></el-switch>
                    </el-form-item>
                    <el-form-item label="磅体1NVR用户名" prop="scale1_nvr_login_username">
                        <el-input v-model="scale_for_edit.scale1.username"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体1NVR密码" prop="scale1_nvr_login_password">
                        <el-input v-model="scale_for_edit.scale1.password"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体1NVRIP" prop="scale1_nvr_ip">
                        <el-input v-model="scale_for_edit.scale1_nvr_ip"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体1通道" prop="scale1_channel">
                        <el-input v-model="scale_for_edit.scale1_channel"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体2NVR用户名" prop="scale2_nvr_login_username">
                        <el-input v-model="scale_for_edit.scale2.username"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体2NVR密码" prop="scale2_nvr_login_password">
                        <el-input v-model="scale_for_edit.scale2.password"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体2NVRIP" prop="scale2_nvr_ip">
                        <el-input v-model="scale_for_edit.scale2_nvr_ip"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体2通道" prop="scale2_channel">
                        <el-input v-model="scale_for_edit.scale2_channel"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体3NVR用户名" prop="scale3_nvr_login_username">
                        <el-input v-model="scale_for_edit.scale3.username"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体3NVR密码" prop="scale3_nvr_login_password">
                        <el-input v-model="scale_for_edit.scale3.password"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体3NVRIP" prop="scale3_nvr_ip">
                        <el-input v-model="scale_for_edit.scale3_nvr_ip"></el-input>
                    </el-form-item>
                    <el-form-item label="磅体3通道" prop="scale3_channel">
                        <el-input v-model="scale_for_edit.scale3_channel"></el-input>
                    </el-form-item>
                    <el-form-item>
                        <el-button type="primary" @click="edit_scale">确认</el-button>
                    </el-form-item>
                </el-form>
            </el-dialog>
            <el-dialog :title="cur_opt_gate.name + '设备操作'" :visible.sync="show_gate_operate" width="60%">
                <vue-grid align="stretch" justify="start">
                    <vue-cell width="6of12">
                        <id-qr-read device_name="入口设备" :id_reader_ip="cur_opt_gate.entry_id_reader_ip" :qr_reader_ip="cur_opt_gate.entry_qr_iq"></id-qr-read>
                        <id-qr-read device_name="出口设备" :id_reader_ip="cur_opt_gate.exit_id_reader_ip" :qr_reader_ip="cur_opt_gate.exit_qr_iq"></id-qr-read>
                    </vue-cell>
                    <vue-cell width="6of12">
                        <gate-device-ctrl :entry="cur_opt_gate.entry_config.cam_ip" :exit="cur_opt_gate.exit_config.cam_ip" :entry_led="cur_opt_gate.entry_config.led_ip" :exit_led="cur_opt_gate.exit_config.led_ip"></gate-device-ctrl>
                    </vue-cell>
                </vue-grid>
            </el-dialog>
            <el-dialog :title="cur_opt_scale.name + '设备操作'" :visible.sync="show_scale_operate" width="60%">
                <vue-grid align="stretch" justify="start">
                    <vue-cell width="6of12">
                        <el-card>
                            <el-row type="flex" :gutter="10" align="center">
                                <el-col :span="10">光栅1</el-col>
                                <el-col :span="14">{{cur_opt_scale.raster1_block?'阻挡':'未阻挡'}}</el-col>
                            </el-row>
                            <el-row type="flex" :gutter="10" align="center">
                                <el-col :span="10">光栅2</el-col>
                                <el-col :span="14">{{cur_opt_scale.raster2_block?'阻挡':'未阻挡'}}</el-col>
                            </el-row>
                            <el-row type="flex" :gutter="10" align="center">
                                <el-col :span="10">当前称重</el-col>
                                <el-col :span="14">{{cur_opt_scale.current_weight}}</el-col>
                            </el-row>
                            <div slot="header">
                                <span>光栅和衡器</span>
                                <el-button style="float: right; padding: 3px 3px" type="warning" @click="get_raster_status">获取</el-button>
                            </div>
                        </el-card>
                    </vue-cell>
                    <vue-cell width="6of12">
                        <gate-device-ctrl :entry="cur_opt_scale.entry_config.cam_ip" :exit="cur_opt_scale.exit_config.cam_ip" :entry_led="cur_opt_scale.entry_config.led_ip" :exit_led="cur_opt_scale.exit_config.led_ip"></gate-device-ctrl>
                    </vue-cell>
                    <vue-cell width="6of12">
                        <el-card>
                            <el-input type="textarea" :rows="5" placeholder="待打印内容" v-model="content_need_print1">
                            </el-input>
                            <div slot="header">
                                <span>入口打印机</span>
                                <el-button style="float: right; padding: 3px 3px" type="warning" @click="print_content1">打印</el-button>
                            </div>
                        </el-card>
                    </vue-cell>
                    <vue-cell width="6of12">
                        <el-card>
                            <el-input type="textarea" :rows="5" placeholder="待打印内容" v-model="content_need_print2">
                            </el-input>
                            <div slot="header">
                                <span>出口打印机</span>
                                <el-button style="float: right; padding: 3px 3px" type="warning" @click="print_content2">打印</el-button>
                            </div>
                        </el-card>
                    </vue-cell>
                    <vue-cell width="6of12">
                        <id-qr-read device_name="入口设备" :id_reader_ip="cur_opt_scale.entry_id_reader_ip" :qr_reader_ip="cur_opt_gate.entry_qr_iq"></id-qr-read>
                    </vue-cell>
                    <vue-cell width="6of12">
                        <id-qr-read device_name="出口设备" :id_reader_ip="cur_opt_scale.exit_id_reader_ip" :qr_reader_ip="cur_opt_gate.exit_qr_iq"></id-qr-read>
                    </vue-cell>
                </vue-grid>
            </el-dialog>
        </el-tab-pane>
        <el-tab-pane label="规则和维护" name="system_info">
            <el-divider>派车规则</el-divider>
            <el-switch v-model="device_config.auto_order" @change="change_auto_order" active-text="自由过车" inactive-text="严格过车">
            </el-switch>
            <el-divider direction="vertical"></el-divider>
            <el-switch v-model="auto_confirm" @change="change_auto_confirm" active-text="自动确认" inactive-text="手动确认">
            </el-switch>
            <div v-if="register_time_config.enabled">
                <el-divider>排号配置</el-divider>
                <el-row :gutter="20">
                    <el-col :span="12">
                        <el-form ref="address_form" :model="address_info" label-width="80px">
                            <el-form-item label="排号最远距离">
                                <el-input v-model="address_info.distance"></el-input>
                            </el-form-item>
                            <el-form-item label="厂区位置">
                                <el-col :span="11">
                                    <el-input v-model="address_info.x" placeholder="经度"></el-input>
                                </el-col>
                                <el-col class="line" :span="2">-</el-col>
                                <el-col :span="11">
                                    <el-input v-model="address_info.y" placeholder="纬度"></el-input>
                                </el-col>
                            </el-form-item>
                            <el-form-item>
                                <el-button type="primary" @click="save_address_info">保存</el-button>
                            </el-form-item>
                        </el-form>
                    </el-col>
                    <el-col :span="12">
                        <el-form ref="time_config_form" :model="register_time_config" label-width="80px">
                            <el-form-item label="过号时间">
                                <el-input v-model="register_time_config.pass_time"></el-input>
                            </el-form-item>
                            <el-form-item label="最短离场排号间隔">
                                <el-input v-model="register_time_config.check_in_time"></el-input>
                            </el-form-item>
                            <el-form-item>
                                <el-button type="primary" @click="save_register_info">保存</el-button>
                            </el-form-item>
                        </el-form>

                    </el-col>
                </el-row>
            </div>
            <el-divider>指引图片</el-divider>
            <vue-grid align="stretch" justify="start">
                <vue-cell width="2of12" v-for="(single_img,index) in  all_prompt_img" :key="index">
                    <el-image :src="$remote_file_url + single_img.attachment_path" fit="fill"></el-image>
                    <el-button type="text" @click="remove_promp_img(single_img)">删除</el-button>
                </vue-cell>
                <el-upload :show-file-list="false" accept="image/*" :action="$remote_url + '/upload/'" :limit="1" :on-success="upload_prompt_image">
                    <el-button size="small" type="primary">点击上传</el-button>
                </el-upload>
            </vue-grid>
            <el-divider>系统更新</el-divider>
            <el-row :gutter="10" type="flex" justify="start">
                <el-col :span="18">
                    <div>当前版本: {{current_version}}</div>
                </el-col>
                <el-col :span="3">
                    <el-button size="small" type="danger" @click="reboot_system">重启系统</el-button>
                </el-col>
                <el-col :span="3">
                    <el-upload :action="$remote_url + '/upload/'" :limit="1" :on-success="confirm_update">
                        <el-button size="small" type="primary">系统更新</el-button>
                        <div slot="tip" class="el-upload__tip">点击上传官方发布的升级包</div>
                    </el-upload>
                </el-col>
            </el-row>
        </el-tab-pane>
        <el-tab-pane label="插件配置" name="plugin_conf">
            <vue-grid align="stretch" justify="start">
                <vue-cell v-for="(single_plugin, index) in installed_plugins" :key="index" width="6of12">
                    <el-card>
                        <div slot="header" class="clearfix">
                            <span>{{single_plugin}}</span>
                            <el-button type="text" style="float:right;" @click="uninstall_plugin(single_plugin)">卸载</el-button>
                        </div>
                        <component :is="get_component_from_plugin_name(single_plugin)"></component>
                    </el-card>

                </vue-cell>
                <vue-cell width="6of12">
                    <el-upload :action="$remote_url + '/upload/'" :limit="1" :on-success="install_plugin" :show-file-list="false">
                        <i class="el-icon-plus avatar-uploader-icon"></i>
                    </el-upload>
                </vue-cell>
            </vue-grid>
            <el-row>
                <el-col :span="20">
                    <el-divider content-position="left">插件队列</el-divider>
                </el-col>
                <el-col :span="4">
                    <el-button type="danger" size="small" @click="pop_plugin_que">弹出</el-button>
                </el-col>
            </el-row>
            <div v-for="(single_item, index) in plugin_que" :key="index">{{single_item}}</div>
        </el-tab-pane>
    </el-tabs>
</div>
</template>

<script>
import GateDeviceCtrl from '../components/GateDeviceCtrl.vue'
import IdQrRead from '../components/IdQrRead.vue'
import ItemForSelect from "../components/ItemForSelect.vue"
import {
    VueGrid,
    VueCell
} from 'vue-grd';
export default {
    name: 'SystemManagement',
    components: {
        "gate-device-ctrl": GateDeviceCtrl,
        "item-for-select": ItemForSelect,
        "id-qr-read": IdQrRead,
        VueGrid,
        VueCell
    },
    data: function () {
        return {
            plugin_que: [],
            address_info: {
                x: 0,
                y: 0,
                distance: 0,
            },
            register_time_config: {
                pass_time: 0,
                check_in_time: 0,
                enabled: false,
            },
            auto_confirm: false,
            get_component_from_plugin_name: function (_plugin_name) {
                var comp = require(`../components/${_plugin_name}.vue`);
                return comp.default;
            },
            all_prompt_img: [],
            installed_plugins: [],
            content_need_print1: '',
            content_need_print2: '',
            cur_opt_scale: {
                entry_config: {},
                exit_config: {}
            },
            cur_opt_gate: {
                entry_config: {},
                exit_config: {}
            },
            show_scale_operate: false,
            show_gate_operate: false,
            activeName: 'device_config',
            current_version: '',
            device_config: {},
            gate_for_edit: {
                entry_config: {},
                exit_config: {},
                entry_login: {},
                exit_login: {},
            },
            scale_for_edit: {
                raster_ip: ['', ''],
                entry_config: {},
                exit_config: {},
                entry_login: {},
                exit_login: {},
                scale1: {},
                scale2: {},
                scale3: {},
            },
            rules: {
                name: [{
                    required: true,
                    message: "请输入名称"
                }],
                entry_cam_ip: [{
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                exit_cam_ip: [{
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                entry_led_ip: [{
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                exit_led_ip: [{
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                entry_id_reader_ip: [{
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                exit_id_reader_ip: [{
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                scale_ip: [{
                    required: true,
                    message: "请输入衡器IP"
                }, {
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                raster_ip: [{
                    required: true,
                    message: "请输入光栅IP"
                }, {
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                entry_printer_ip: [{
                    required: true,
                    message: "请输入打印机IP"
                }, {
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
                exit_printer_ip: [{
                    required: true,
                    message: "请输入打印机IP"
                }, {
                    pattern: /([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))(\.([0,1]?\d{1,2}|2([0-4][0-9]|5[0-5]))){3}/g,
                    message: '请输入正确的IP'
                }],
            },
            show_edit_gate_config: false,
            show_edit_scale_config: false,
        };
    },
    methods: {
        init_plugin_que: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "go_through_plugin_que", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.plugin_que = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.plugin_que, index, element);
                });
            });
        },
        pop_plugin_que: function () {
            var vue_this = this;
            vue_this.$dialog.confirm({
                title: '弹出确认',
                message: '确定要弹出该任务吗？'
            }).then(function () {
                vue_this.$call_remote_process("vehicle_order_center", "cancel_plugin_que", [vue_this.$cookies.get("zh_ssid")]).then(function () {
                    vue_this.init_plugin_que();
                });
            });
        },
        get_register_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_register_info", []).then(function (resp) {
                vue_this.register_time_config = resp;
            });
        },
        save_register_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "set_register_info", [vue_this.$cookies.get("zh_ssid"), {
                enabled: true,
                pass_time: parseInt(vue_this.register_time_config.pass_time),
                check_in_time: parseInt(vue_this.register_time_config.check_in_time),
            }]).then(function (resp) {
                if (resp) {
                    vue_this.get_register_info();
                }
            });
        },
        save_address_info: function () {
            var vue_this = this;
            var set_arg = {
                x: parseFloat(vue_this.address_info.x),
                y: parseFloat(vue_this.address_info.y),
                distance: parseFloat(vue_this.address_info.distance)
            };
            vue_this.$call_remote_process("system_management", "set_company_address_info", [vue_this.$cookies.get("zh_ssid"), set_arg]).then(function (resp) {
                if (resp) {
                    vue_this.get_company_address_info();
                }
            });
        },
        get_company_address_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "get_company_address_info", []).then(function (resp) {
                vue_this.address_info = resp;
            });
        },
        remove_promp_img: function (_image) {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "delete_prompt_image", [vue_this.$cookies.get("zh_ssid"), _image.id]).then(function (resp) {
                if (resp) {
                    vue_this.init_prompt_img();
                }
            });

        },
        upload_prompt_image: function (resp, file) {
            var real_path = resp.match(/^\/tmp\/.*/gm)[0];
            var file_name_split = file.name.split('.');
            var att_path = real_path + '.' + file_name_split[file_name_split.length - 1];
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "upload_prompt_image", [vue_this.$cookies.get("zh_ssid"), att_path]).then(function (resp) {
                if (resp) {
                    vue_this.init_prompt_img();
                }
            });
        },
        install_plugin(resp, file) {
            var real_path = resp.match(/^\/tmp\/.*/gm)[0];
            var file_name_split = file.name.split('.');
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "install_plugin", [vue_this.$cookies.get("zh_ssid"), file_name_split[0], real_path]).then(function (resp) {
                if (resp) {
                    vue_this.init_plugins();
                }
            });
        },
        uninstall_plugin: function (_plugin) {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "uninstall_plugin", [vue_this.$cookies.get("zh_ssid"), _plugin]).then(function () {
                vue_this.init_plugins();
            })
        },
        reboot_system: function () {
            var vue_this = this;
            this.$confirm('确定要重启吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                this.$call_remote_process("system_management", "reboot_system", [vue_this.$cookies.get("zh_ssid")]).finally(() => {
                    vue_this.$alert('请稍后刷新页面', '正在重启', {
                        confirmButtonText: '确定',
                    });
                });
            });
        },
        confirm_update: function (resp) {
            var vue_this = this;
            var real_path = resp.match(/^\/tmp\/.*/gm)[0];
            this.$confirm('确定要更新吗', '提示', {
                confirmButtonText: '确定',
                cancelButtonText: '取消',
                type: 'warning'
            }).then(() => {
                vue_this.$call_remote_process("system_management", "run_update", [vue_this.$cookies.get("zh_ssid"), real_path]).finally(function () {
                    vue_this.$alert('请稍后刷新页面', '正在更新', {
                        confirmButtonText: '确定',
                    });
                });
            });
        },
        open_gate_operate: function (_gate) {
            this.cur_opt_gate = {
                ..._gate
            };
            this.show_gate_operate = true;
        },
        get_id_no: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "read_id_no", [vue_this.cur_opt_gate.entry_id_reader_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_gate, "entry_id_no", resp);
            });
            vue_this.$call_remote_process("system_management", "read_id_no", [vue_this.cur_opt_gate.exit_id_reader_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_gate, "entry_id_no", resp);
            });
            vue_this.$call_remote_process("system_management", "read_id_no", [vue_this.cur_opt_scale.entry_id_reader_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, "entry_id_no", resp);
            });
            vue_this.$call_remote_process("system_management", "read_id_no", [vue_this.cur_opt_scale.exit_id_reader_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, "exit_id_no", resp);
            });
        },
        print_content1: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "print_content", [vue_this.cur_opt_scale.entry_printer_ip, vue_this.content_need_print1]).then(function (resp) {
                if (resp) {
                    vue_this.$message({
                        message: "打印成功",
                        type: 'success',
                    });
                } else {
                    vue_this.$message({
                        message: "打印失败",
                        type: 'error',
                    });
                }
            });
        },
        print_content2: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "print_content", [vue_this.cur_opt_scale.exit_printer_ip, vue_this.content_need_print2]).then(function (resp) {
                if (resp) {
                    vue_this.$message({
                        message: "打印成功",
                        type: 'success',
                    });
                } else {
                    vue_this.$message({
                        message: "打印失败",
                        type: 'error',
                    });
                }
            });
        },
        get_raster_status: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "raster_is_block", [vue_this.cur_opt_scale.raster_ip[0]]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, 'raster1_block', resp);
            });
            vue_this.$call_remote_process("system_management", "raster_is_block", [vue_this.cur_opt_scale.raster_ip[1]]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, 'raster2_block', resp);
            });
            vue_this.$call_remote_process("system_management", "read_scale", [vue_this.cur_opt_scale.scale_ip]).then(function (resp) {
                vue_this.$set(vue_this.cur_opt_scale, 'current_weight', resp);
            });
        },
        open_scale_operate: function (_scale) {
            this.show_scale_operate = true;
            this.cur_opt_scale = {
                ..._scale
            };
        },
        change_auto_order: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "edit_device_config", [vue_this.$cookies.get("zh_ssid"), vue_this.device_config]).then(function (resp) {
                if (resp) {
                    vue_this.init_device_info();
                }
            });
        },
        edit_gate: function () {
            var vue_this = this;
            vue_this.gate_for_edit.entry_channel = parseInt(vue_this.gate_for_edit.entry_channel);
            vue_this.gate_for_edit.exit_channel = parseInt(vue_this.gate_for_edit.exit_channel);
            this.$refs.edit_gate_form.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("system_management", "edit_device_config", [vue_this.$cookies.get("zh_ssid"), vue_this.device_config]).then(function (resp) {
                        if (resp) {
                            vue_this.show_edit_gate_config = false;
                        }
                    });
                }
            });
        },
        edit_scale: function () {
            var vue_this = this;
            vue_this.scale_for_edit.entry_channel = parseInt(vue_this.scale_for_edit.entry_channel);
            vue_this.scale_for_edit.exit_channel = parseInt(vue_this.scale_for_edit.exit_channel);
            vue_this.scale_for_edit.coefficient = parseFloat(vue_this.scale_for_edit.coefficient);
            vue_this.scale_for_edit.scale1_channel = parseInt(vue_this.scale_for_edit.scale1_channel);
            vue_this.scale_for_edit.scale2_channel = parseInt(vue_this.scale_for_edit.scale2_channel);
            vue_this.scale_for_edit.scale3_channel = parseInt(vue_this.scale_for_edit.scale3_channel);
            vue_this.$refs.edit_scale_form.validate((valid) => {
                if (valid) {
                    vue_this.$call_remote_process("system_management", "edit_device_config", [vue_this.$cookies.get("zh_ssid"), vue_this.device_config]).then(function (resp) {
                        if (resp) {
                            vue_this.show_edit_scale_config = false;
                        }
                    });
                }
            });
        },
        open_gate_edit: function (_gate) {
            this.gate_for_edit = _gate;
            this.show_edit_gate_config = true;
        },
        open_scale_edit: function (_scale) {
            this.scale_for_edit = _scale;
            this.show_edit_scale_config = true;
        },
        init_device_info: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", 'get_device_config', [vue_this.$cookies.get('zh_ssid')]).then(function (resp) {
                vue_this.device_config = resp;
            });
        },
        init_plugins: function () {
            var vue_this = this;
            vue_this.$call_remote_process("plugin_management", "get_installed_plugins", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                vue_this.installed_plugins = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.installed_plugins, index, element);
                });
            });
        },
        change_auto_confirm: function () {
            var vue_this = this;
            vue_this.$call_remote_process("system_management", "set_auto_confirm", [vue_this.$cookies.get("zh_ssid"), vue_this.auto_confirm]);
        },
        init_prompt_img: function () {
            var vue_this = this;

            vue_this.$call_remote_process("system_management", "get_all_prompt_image", []).then(function (resp) {
                vue_this.all_prompt_img = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.all_prompt_img, index, element);
                });
            });
        },
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("system_management", 'current_version', []).then(function (resp) {
            vue_this.current_version = resp;
        });
        this.init_device_info();
        this.init_plugins();
        vue_this.$call_remote_process("system_management", "is_auto_confirm", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
            vue_this.auto_confirm = resp;
        });
        this.init_prompt_img();
        this.get_company_address_info();
        this.get_register_info();
        this.init_plugin_que();

    },
}
</script>

<style scoped>
.device_config_show {
    border: 1px solid black;
    margin-top: 10px;
    padding: 5px;
}
</style>
