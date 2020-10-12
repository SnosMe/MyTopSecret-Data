import { shallowReactive } from 'vue'

export const globalState = shallowReactive({
  encrypted: null as Uint8Array | null
})
