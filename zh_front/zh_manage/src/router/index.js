import Vue from 'vue'
import VueRouter from 'vue-router'

Vue.use(VueRouter)

const routes = [
  {
    path: '/',
    name: 'Home',
    component: () => import('../views/Home.vue')
  },
  {
    path: '/system_management',
    name: 'SystemManagement',
    component: () => import('../views/SystemManagement.vue')
  },
  {
    path: '/user_management',
    name: 'UserManagement',
    component: () => import('../views/UserManagement.vue')
  },
  {
    path: '/contract_management',
    name: 'ContractManagement',
    component: () => import('../views/ContractManagement.vue'),
  },
  {
    path: '/stuff_management',
    name: 'StuffManagement',
    component: () => import('../views/StuffManagement.vue')
  },{
    path: '/video_stream',
    name: 'VideoStream',
    component: () => import('../views/VideoStream.vue')
  },
  {
    path: '/vehicle_management',
    name: 'VehicleManagement',
    component: () => import('../views/VehicleManagement.vue')
  },
  {
    path: '/vehicle_order_center',
    name: 'VehicleOrderCenter',
    component: () => import('../views/VehicleOrderCenter.vue')
  },
  {
    path: '/white_record',
    name: 'WhiteRecord',
    component: () => import('../views/WhiteRecord.vue')
  },
  {
    path: '/mobile/field_opt/:order_no',
    name: 'FieldOpt',
    component: () => import('../views/FieldOpt.vue'),
    meta: {
      mobile: true,
    },
  },
{
    path: '/mobile/scale_state',
    name: 'ScaleState',
    component: () => import('../views/ScaleState.vue'),
    meta: {
      mobile: true,
    },
  },
  {
    path: '/mobile/all_came_vehicle',
    name: 'AllCameVehicle',
    component: () => import('../views/AllCameVehicle.vue'),
    meta: {
      mobile: true,
    },
  },
  {
    path: '/mobile/contract_management',
    name: 'ContractManagement',
    component: () => import('../views/ContractManagement.vue'),
    meta: {
      mobile: true,
    },
  },
  {
    path: '/vehicle_detail/:order_no',
    name: 'VehicleDetail',
    component: () => import('../views/VehicleDetail.vue'),
    meta: {
      is_sub: true,
      subtitle: '派车单',
    },
  }, {
    path: '/mobile/vehicle_order_center',
    name: 'MobileVehicleOrderCenter',
    component: () => import('../views/VehicleOrderCenter.vue'),
    meta: {
      mobile: true,
    },
  },
  {
    path: '/mobile/vehicle_management',
    name: 'MobileVehicleManagement',
    component: () => import('../views/VehicleManagement.vue'),
    meta: {
      mobile: true,
    },
  }, {
    path: '/mobile/vehicle_detail/:order_no',
    name: 'MobileVehicleDetail',
    component: () => import('../views/VehicleDetail.vue'),
    meta: {
      mobile: true,
    },
  }, {
    path: '/mobile/self_order',
    name: 'DriverSelfOpt',
    component: () => import('../views/DriverSelfOpt.vue'),
    meta: {
      mobile: true,
      no_need_login: true,
      no_tabbar: true,
    },
  }, {
    path: '/mobile/self_order_opt',
    name: 'SelfOrderOpt',
    component: () => import('../views/SelfOrderOpt.vue'),
    meta: {
      mobile: true,
    },
  },{
    path: '/mobile/ticket/:order_number',
    name: 'TicketPage',
    component: () => import('../views/TicketPage.vue'),
    meta: {
      mobile: true,
      no_need_login: true,
      no_tabbar: true,
    },
  },
  {
    path: '/mobile/white_scale',
    name: 'WhiteScale',
    component: () => import('../views/WhiteScale.vue'),
    meta: {
      mobile: true,
      no_need_login: true,
      no_tabbar: true,
    },
  },
]

const router = new VueRouter({
  routes
})

export default router
