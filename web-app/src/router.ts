import { createRouter, createWebHashHistory, RouteRecordRaw } from 'vue-router'
import Decrypt from './components/Decode.vue'
import Encrypt from './components/Encode.vue'
import Prettify from './components/Prettify.vue'

const routes: Array<RouteRecordRaw> = [
  {
    path: '/decrypt',
    component: Decrypt
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
