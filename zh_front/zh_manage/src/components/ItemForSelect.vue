<template>
<div class="item_for_select_show">
    <el-autocomplete v-if="!$route.meta.mobile" :disabled="disabled" v-model="input_value" :fetch-suggestions="proc_match" placeholder="请输入内容"></el-autocomplete>
    <van-field v-else readonly clickable :rules="rules" :value="value_show" :label="label" :placeholder="'点击选择' + label" @click="show_picker = true" />
    <van-popup get-container="#app" v-model="show_picker" position="bottom">
        <van-picker show-toolbar :columns="sel_col" @confirm="onConfirm" @cancel="show_picker = false" />
    </van-popup>
</div>
</template>

<script>
import PinyinMatch from "pinyin-match"
export default {
    name: 'ItemForSelect',
    data: function () {
        return {
            all_item: [],
            show_picker: false,
            value_show: '',
        };
    },
    props: {
        value: String,
        search_key: String,
        disabled: Boolean,
        rules: [],
        label: String,
        related_contract: String,
    },
    model: {
        prop: 'value',
        event: 'change',
    },
    computed: {
        sel_col: function () {
            var ret = [];
            this.all_item.forEach(element => {
                ret.push(element.value);
            });
            return ret;
        },
        input_value: {
            get() {
                return this.value;
            },
            set(_value) {
                this.$emit("change", _value)
            },
        },
    },
    methods: {
        onConfirm(value) {
            this.value_show = value;
            this.$emit("change", value)
            this.show_picker = false;
        },
        proc_match: function (_query, cb) {
            var ret = this.all_item;
            if (_query) {
                ret = [];
                this.all_item.forEach(element => {
                    if (PinyinMatch.match(element.value, _query)) {
                        ret.push(element);
                    }
                });
            }
            cb(ret)
        },
    },
    beforeMount: async function () {
        var vue_this = this;
        switch (vue_this.search_key) {
            case 'company_name':
                vue_this.$call_remote_process("contract_management", "get_all_contract", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                    resp.forEach(element => {
                        vue_this.all_item.push({
                            value: element.name
                        });
                    });
                });
                break;
            case 'stuff_name':
                vue_this.$call_remote_process("stuff_management", "get_all_stuff", [vue_this.$cookies.get("zh_ssid"), vue_this.related_contract]).then(function (resp) {
                    resp.forEach(element => {
                        vue_this.all_item.push({
                            value: element.name
                        });
                    });
                });

                break;
            case 'scale_brand':
                vue_this.$call_remote_process("system_management", "get_all_scale_brand", []).then(function (resp) {
                    resp.forEach(element => {
                        vue_this.all_item.push({
                            value: element
                        });
                    });
                });

                break;
            case 'company_address':
                vue_this.$call_remote_process("contract_management", "get_all_contract", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                    resp.forEach(element => {
                        vue_this.all_item.push({
                            value: element.company_address,
                        });
                    });
                });
                break;
            case 'use_for':
                vue_this.all_item.push({
                    value: "煤场用煤"
                });
                vue_this.all_item.push({
                    value: "工业用煤"
                });
                vue_this.all_item.push({
                    value: "民用燃煤"
                });
                vue_this.all_item.push({
                    value: "铁路用煤"
                });
                vue_this.all_item.push({
                    value: "电厂用煤"
                });
                vue_this.all_item.push({
                    value: "化工用煤"
                });
                vue_this.all_item.push({
                    value: "供暖用煤"
                });
                break;
            default:
                break;
        }
    },
}
</script>

<style>

</style>
