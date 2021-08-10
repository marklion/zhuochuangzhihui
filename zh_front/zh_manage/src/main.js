import Vue from 'vue'
import App from './App.vue'
import './plugins/element.js'
import router from './router'
import {
  get_client, call_remote_process, call_remote_process_no_toast
} from '@/plugins/rpc_helper.js'

Vue.config.productionTip = false
Vue.prototype.$get_client = get_client;
Vue.prototype.$call_remote_process = call_remote_process;
Vue.prototype.$call_remote_process_no_toast = call_remote_process_no_toast;

new Vue({
  router,
  render: h => h(App)
}).$mount('#app')
