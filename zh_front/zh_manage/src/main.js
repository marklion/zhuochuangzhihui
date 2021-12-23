import Vue from 'vue'
import App from './App.vue'
import './plugins/element.js'
import router from './router'
import cookies from 'vue-cookies'
import './style/main.css';
import VCharts from 'v-charts'

Vue.use(VCharts)
import {
  get_client, call_remote_process, call_remote_process_no_toast
} from '@/plugins/rpc_helper.js'

import store from './store'
Vue.config.productionTip = false
Vue.prototype.$get_client = get_client;
Vue.prototype.$call_remote_process = call_remote_process;
Vue.prototype.$call_remote_process_no_toast = call_remote_process_no_toast;
Vue.prototype.$cookies = cookies;
Vue.prototype.$remote_url = process.env.VUE_APP_BACK_END_URL;
Vue.prototype.$remote_file_url = process.env.VUE_APP_BACK_END_URL + "/logo_res/";

new Vue({
  router,
  store,
  render: h => h(App)
}).$mount('#app')
