<template>
<div id="app">
    <el-row>
        <el-col :span="6">
            <el-menu default-active="Home" router>
                <el-menu-item index="Home" :route="{name:'Home'}">概览</el-menu-item>
                <el-menu-item index="SystemManagement" :route="{name:'SystemManagement'}">系统设置</el-menu-item>
            </el-menu>
        </el-col>
        <el-col :span="18">
            <router-view></router-view>
        </el-col>
    </el-row>

</div>
</template>

<script>
export default {

}
</script>

<style>

</style>
