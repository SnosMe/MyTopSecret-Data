<template>
  <navbar />
  <page-content>
    <button @click="toggleHelp" class="text-sm text-gray-700 mb-2">Document format reference</button>
    <div v-if="showHelp" class="mb-2 border border-gray-200 rounded p-2">
      <p>Document consists of records separated by three dashes.</p>
      <p>Each record has multiple key-value pairs. Key and value are separated by a colon. Keys can be repeated.
         The value is a text, either on same line or starts on next line indented by 2 spaces.</p>
      <p>Supported keys: <i>{{ supportedKeys }}</i>.</p>
    </div>
    <form>
      <textarea placeholder="password: example&#10;---&#10;text:&#10;  multiline&#10;  example"
        class="border w-full font-mono leading-snug p-2 rounded shadow-inner mb-1"
        spellcheck="false" rows="8"
        v-model="rawText" />
      <div class="flex items-center justify-between flex-wrap -m-1">
        <input type="password"
          placeholder="Secret"
          :disabled="isEncrypting"
          class="border shadow-inner rounded px-2 min-w-0 m-1 flex-grow max-w-xs"
          v-model="secret" />
        <button class="rounded px-2 text-white m-1"
          :disabled="isEncrypting"
          :class="!isEncrypting ? 'bg-gray-700' : 'bg-gray-400'"
          @click.prevent="encrypt"
          >Encrypt</button>
      </div>
      <div v-if="encryptError"
        class="bg-red-200 rounded text-red-700 px-2 mt-2 inline-block">{{ encryptError }}</div>
    </form>
  </page-content>
</template>

<script lang="ts">
import { defineComponent, shallowRef, watch } from 'vue'
import { thread } from '@/worker/interface'
import Navbar from './Navbar.vue'
import PageContent from './PageContent.vue'
import { globalState } from '@/util/global'
import { KEYID } from '../worker/native'
import router from '@/router'

export default defineComponent({
  components: { Navbar, PageContent },
  setup () {
    const rawText = shallowRef('')
    const secret = shallowRef('')
    const isEncrypting = shallowRef(false)
    const encryptError = shallowRef<string | null>(null)
    const showHelp = shallowRef(false)

    async function encrypt () {
      isEncrypting.value = true

      try {
        const secretText = secret.value
        secret.value = ''
        globalState.encrypted = await thread.mtsdEncrypt(rawText.value, secretText)
        await router.push('/prettify')
      } catch (e) {
        const err = await thread.getLastError()
        encryptError.value = err.message || e.message
      } finally {
        isEncrypting.value = false
      }
    }

    function toggleHelp () {
      showHelp.value = !showHelp.value
    }

    watch(rawText, () => {
      encryptError.value = null
    })

    return {
      rawText,
      isEncrypting,
      secret,
      encrypt,
      encryptError,
      showHelp,
      toggleHelp,
      supportedKeys: KEYID.slice(1).sort().join(', ')
    }
  }
})
</script>
