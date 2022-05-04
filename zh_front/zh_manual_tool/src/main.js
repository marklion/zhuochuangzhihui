import Vue from 'vue'
import App from './App.vue'
import './plugins/element.js'
import cookies from 'vue-cookies'

Vue.config.productionTip = false

Vue.prototype.$remote_url = process.env.VUE_APP_BACK_END_URL;
Vue.prototype.$cookies = cookies;
Vue.prototype.$make_time_string = function(date, _splite) {
  var y = date.getFullYear();
  var m = date.getMonth() + 1;
  m = m < 10 ? ('0' + m) : m;
  var d = date.getDate();
  d = d < 10 ? ('0' + d) : d;
  var h = date.getHours();
  h = h < 10 ? ('0' + h) : h;
  var minute = date.getMinutes();
  minute = minute < 10 ? ('0' + minute) : minute;
  var second = date.getSeconds();
  second = second < 10 ? ('0' + second) : second;
  return y + _splite + m + _splite + d + ' ' + h + ':' + minute + ':' + second;
};

new Vue({
  render: h => h(App),
}).$mount('#app')
