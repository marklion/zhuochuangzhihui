<template>
<div class="white_scale_show">
    <van-dialog v-model="init_number_diag" title="输入车牌号查询" :showConfirmButton="false" closeOnClickOverlay>
        <van-form @submit="search_vehicle">
            <van-field v-model="search_number" label="车牌号" placeholder="无需完整车牌号，数字即可" :rules="[{ required: true, message: '请输入车牌' }]" />
            <div style="margin: 16px;">
                <van-button block type="info" native-type="submit">查询</van-button>
            </div>
        </van-form>
    </van-dialog>
    <van-dialog v-model="select_number_diag" title="选择车辆" :showConfirmButton="false" closeOnClickOverlay>
        <van-cell v-for="(single_vehicle, index) in record_for_select" :key="index" :title="single_vehicle" @click="open_set_diag(single_vehicle)"></van-cell>
    </van-dialog>

    <van-dialog v-model="set_diag" title="设置物料" :showConfirmButton="false" closeOnClickOverlay>
        <van-form @submit="set_stuff_vehicle">
            <van-field readonly clickable :value="set_stuff" :label="set_number" placeholder="点击选择物料" @click="showPicker = true" :rules="[{ required: true, message: '请选择物料' }]" />
            <van-popup v-model="showPicker" position="bottom" get-container="body">
                <van-picker show-toolbar :columns="stuff_for_select" @confirm="onConfirm" @cancel="showPicker = false" />
            </van-popup>
            <div style="margin: 16px;">
                <van-button block type="info" native-type="submit">确定</van-button>
            </div>
        </van-form>
    </van-dialog>

    <van-dialog v-model="qr_show" title="过磅码" :showConfirmButton="false">
        <div style="text-align:center;">
            <vue-qr :text="'http:///#/mobile/self_order/v' + qr_string"></vue-qr>
            <div>
                此码对准地磅扫描器
            </div>
        </div>
    </van-dialog>
</div>
</template>

<script>
import PinyinMatch from "pinyin-match"
import vueQr from 'vue-qr'
export default {
    name: 'WhiteScaleShow',
    data: function () {
        return {
            qr_show: false,
            init_number_diag: false,
            search_number: '',
            record: [],
            select_number_diag: false,
            record_for_select: [],
            set_diag: false,
            set_number: '',
            set_stuff: '',
            stuff_for_select: [],
            showPicker: false,
            qr_string: '',
        };
    },
    components: {
        vueQr
    },
    methods: {
        onConfirm(value) {
            this.set_stuff = value;
            this.showPicker = false;
        },
        init_select_pool: function () {
            var vue_this = this;
            vue_this.$call_remote_process("stuff_management", "get_white_list_stuff", []).then(function (resp) {
                vue_this.stuff_for_select = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.stuff_for_select, index, element);
                });
            });
        },
        init_all_white: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_management", "get_white_vehicle", []).then(function (resp) {
                vue_this.record = [];
                resp.forEach((element, index) => {
                    vue_this.$set(vue_this.record, index, element);
                });
                vue_this.init_number_diag = true;
            });
        },
        search_vehicle: function () {
            if (this.search_number) {
                this.record_for_select = [];
                this.record.forEach(element => {
                    if (PinyinMatch.match(element.main_vehicle_number, this.search_number)) {
                        this.record_for_select.push(element.main_vehicle_number);
                    }
                });
                if (this.record_for_select.length > 0) {
                    this.init_number_diag = false;
                    this.select_number_diag = true;
                } else {
                    this.$dialog.alert({
                        message: '未找到车辆'
                    });
                }
            }
        },
        open_set_diag: function (_vehicle_number) {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "get_white_vehicle_stuff", [_vehicle_number]).then(function (resp) {
                vue_this.set_stuff = resp;
                vue_this.set_number = _vehicle_number;
                vue_this.select_number_diag = false;
                vue_this.set_diag = true;
            });
        },
        set_stuff_vehicle: function () {
            var vue_this = this;
            vue_this.$call_remote_process("vehicle_order_center", "record_white_vehicle_stuff", [vue_this.set_number, vue_this.set_stuff]).then(function (resp) {
                vue_this.set_diag = false;
                if (resp) {
                    vue_this.qr_string = resp;
                    vue_this.qr_show = true;
                }
            });
        },
    },
    beforeMount: function () {
        this.init_select_pool();
        this.init_all_white();
    },
}
</script>

<style>

</style>
