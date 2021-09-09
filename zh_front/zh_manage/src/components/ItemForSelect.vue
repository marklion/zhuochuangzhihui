<template>
<div class="item_for_select_show">
    <el-autocomplete v-model="input_value" :fetch-suggestions="proc_match" placeholder="请输入内容"></el-autocomplete>
</div>
</template>

<script>
import PinyinMatch from "pinyin-match"
export default {
    name: 'ItemForSelect',
    data: function () {
        return {
            all_item: [],
        };
    },
    props: {
        value: String,
        search_key: String,
    },
    model: {
        prop: 'value',
        event: 'change',
    },
    computed: {
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
                vue_this.$call_remote_process("stuff_management", "get_all_stuff", [vue_this.$cookies.get("zh_ssid")]).then(function (resp) {
                    resp.forEach(element => {
                        vue_this.all_item.push({
                            value: element.name
                        });
                    });
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
