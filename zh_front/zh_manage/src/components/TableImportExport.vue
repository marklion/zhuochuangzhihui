<template>
<div class="table_import_export_show">
    <div class="inline_button_group_show">
        <el-button type="primary" size="mini" v-if="!no_need_import" @click="show_upload = true">导入</el-button>
        <el-button type="success" size="mini" @click="make_download_table(export_table)">导出</el-button>
    </div>
    <el-dialog title="导入表格" :visible.sync="show_upload" width="60%" class="upload_diag_show">
        <el-upload accept=".xls,.xlsx" action="" :show-file-list="false" :limit="1" :http-request="import_table">
            <div slot="tip">
                请点击下载<el-button type="text" @click="make_download_table(sample_table)">范例表格</el-button>后，在范例表格中批量增加要导入的内容并保存，然后再点击上传按钮上传编辑后的范例表格。
            </div>
            <el-button size="mini" type="primary">上传</el-button>
        </el-upload>
    </el-dialog>

</div>
</template>

<script>
import XLSX from 'xlsx';
export default {
    name: 'TableImportExport',
    data: function () {
        return {
            show_upload: false,
        };
    },
    props: {
        export_name: String,
        item_name_map: Object,
        export_table: Array,
        sample_table: Array,
        no_need_import:Boolean,
    },
    methods: {
        import_table: function (_file) {
            var vue_this = this;
            const reader = new FileReader()
            reader.onload = function (e) {
                const data = e.target.result;
                var wb = XLSX.read(data, {
                    type: 'binary'
                });
                var ret = XLSX.utils.sheet_to_json(wb.Sheets['s1'], {raw:false});
                var event_array = [];
                ret.forEach(element => {
                    var event_content = {};
                    Object.keys(element).forEach(element_key => {
                        var key = Object.keys(vue_this.item_name_map).find((key) => {
                            return vue_this.item_name_map[key].text == element_key;
                        });
                        event_content[key] = vue_this.item_name_map[key].parser ? vue_this.item_name_map[key].parser(element[element_key]) : element[element_key];
                    });
                    event_array.push(event_content);
                });
                vue_this.$emit("proc_table", event_array);
            }
            reader.readAsBinaryString(_file.file);
            vue_this.show_upload = false;
        },
        make_download_table: function (_table_ob) {
            var fin_json = [];
            var header = {};
            Object.keys(this.item_name_map).forEach(i => {
                header[i] = this.item_name_map[i].text;
            });
            fin_json.push(header);
            _table_ob.forEach(element => {
                var tmp_record = {};
                Object.keys(header).forEach(key => {
                    tmp_record[key] = this.item_name_map[key].formatter ? this.item_name_map[key].formatter(element[key]) : element[key];
                });
                fin_json.push(tmp_record);
            });
            var sheet = XLSX.utils.json_to_sheet(fin_json, {
                skipHeader: true
            });
            var wb = {
                SheetNames: ["s1"],
                Sheets: {
                    s1: sheet
                }
            };
            XLSX.writeFile(wb, this.export_name);
        },
    },
}
</script>

<style scoped>
.upload_diag_show {
    text-align: left;
}

.table_import_export_show {
    display: inline-block;
    margin: 10px;
}
</style>
