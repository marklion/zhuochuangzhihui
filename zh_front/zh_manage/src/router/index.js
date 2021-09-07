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
    component: () => import('../views/ContractManagement.vue')
  },
  {
    path: '/stuff_management',
    name: 'StuffManagement',
    component: () => import('../views/StuffManagement.vue')
  },
  {
    path: '/vehicle_management',
    name: 'VehicleManagement',
    component: () => import('../views/VehicleManagement.vue')
  },
]

const router = new VueRouter({
  routes
})

export default router
