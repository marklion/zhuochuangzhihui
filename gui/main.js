import App from './App'

// #ifndef VUE3
import Vue from 'vue'
Vue.config.productionTip = false
Vue.prototype.$send_req = function (_url, _data) {
  return new Promise((resolve, reject) => {
    uni.showLoading({ title: '加载中...', mask: true });
    uni.request({
      url: 'https://' + uni.getStorageSync('domain') + '.d8sis.cn' + _url,
      data: _data,
      method: 'POST',
      header: {
        'token': uni.getStorageSync('token'),
      },
      success: (res) => {
        let data = res.data;
        if (data.err_msg.length > 0) {
          uni.showToast({
            title: data.err_msg,
            icon: 'none',
            duration: 2000
          });
          reject(data.err_msg)
        }
        else {
          resolve(data.result)
        }
        uni.hideLoading();
      },
      fail: (res) => {
        reject(res)
        uni.hideLoading();
      }
    })
  })
};
import uView from '@/uni_modules/uview-ui'
Vue.use(uView)
App.mpType = 'app'
const app = new Vue({
  ...App
})
app.$mount()

// #endif

// #ifdef VUE3
import { createSSRApp } from 'vue'
export function createApp() {
  const app = createSSRApp(App)
  return {
    app
  }
}
// #endif