<template>
<div id="app">
    <div class="web_header_show">
        卓创互联
    </div>
    <div >
        <el-row >
            <el-col :span="4">
                <el-menu class="web_nav_show" default-active="Home" router background-color="#545c64" text-color="#fff">
                    <el-menu-item index="Home" :route="{name:'Home'}">概览</el-menu-item>
                    <el-menu-item index="SystemManagement" :route="{name:'SystemManagement'}">系统设置</el-menu-item>
                </el-menu>
            </el-col>
            <el-col :span="20" >
                <router-view></router-view>
            </el-col>
        </el-row>
    </div>

</div>
</template>

<script>
export default {

}
</script>

<style scoped>
.web_header_show {
    height: 60;
    line-height: 60px;
    padding-left: 40px;
    background-color: rgb(114, 57, 20);
    color: bisque;
    font-weight: bold;
    font-size: 40px;
}

.web_nav_show {
    min-height: 600px;
}

</style>
