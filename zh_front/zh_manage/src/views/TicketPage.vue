<template>
<div class="ticket_page_show">
    <van-nav-bar title="电子磅单" left-text="返回" left-arrow @click-left="$router.back()" />
    <div class="ticket_content_show">
        <ticket :order_number="$route.params.order_number">
        </ticket>
        <vue-qr :text="'http://' + domain_name + '/#/mobile/ticket/' + $route.params.order_number"></vue-qr>
    </div>
</div>
</template>

<script>
import Ticket from "../components/Ticket.vue"
import vueQr from 'vue-qr'
export default {
    name: 'TicketPage',
    data: function () {
        return {
            domain_name: '',
        };
    },
    components: {
        "ticket": Ticket,
        vueQr,
    },
    beforeMount: function () {
        var vue_this = this;
        vue_this.$call_remote_process("system_management", "get_domain_name", []).then(function (resp) {
            vue_this.domain_name = resp;
        });
    }
}
</script>

<style scoped>
.ticket_content_show {
    padding: 20px;
}
</style>
