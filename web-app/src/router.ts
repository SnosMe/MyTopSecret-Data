import { createRouter, createWebHashHistory, RouteRecordRaw } from 'vue-router'
import Decrypt from './components/Decrypt.vue'
import Encrypt from './components/Encrypt.vue'
import Prettify from './components/Prettify.vue'
import Scan from './components/Scan.vue'

const routes: Array<RouteRecordRaw> = [
  {
    path: '/decrypt',
    component: Decrypt
  },
  {
    path: '/decrypt/scan',
    component: Scan
  },
  {
    path: '/encrypt',
    component: Encrypt
  },
  {
    path: '/prettify',
    component: Prettify
  }
]

const router = createRouter({
  history: createWebHashHistory(),
  routes
})

export default router
