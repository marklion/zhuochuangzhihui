import Vue from 'vue'
import Vuex from 'vuex'

Vue.use(Vuex)

export default new Vuex.Store({
  state: {
    user_info: {
      name: '',
      phone: '',
      permission: -1,
      id: 0,
      permission_name:'',
      need_change_password:false,
    },

    is_login: false,
  },
  mutations: {
    set_user_info(state, user_info) {
      state.user_info = user_info;
    },
    set_login(state, login) {
      state.is_login = login;
    }
  },
  actions: {
  },
  modules: {
  }
})
