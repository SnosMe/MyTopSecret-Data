<template>
  <navbar>
    <template #secondary>
      <div class="flex -mx-1 mt-1 text-sm">
        <span class="p-1 font-semibold">Text</span>
        <router-link to="/decrypt/scan" class="p-1 ml-2">Scan</router-link>
        <label type="button" class="p-1 ml-2 cursor-pointer" for="decrypt_file">File</label>
        <input @input="fromFile" type="file" id="decrypt_file" class="hidden">
      </div>
    </template>
  </navbar>
  <page-content class="relative">
    <form>
      <div class="absolute top-0 right-0 p-2">
        <div v-if="encoded.isValid !== undefined"
          class="border rounded px-2 leading-tight shadow"
          :class="encoded.isValid ? 'bg-green-200 border-green-300 text-green-700' : 'bg-red-200 border-red-300 text-red-700'"
        >{{ encoded.isValid ? 'Valid' : 'Invalid' }}</div>
      </div>
      <textarea class="border w-full font-mono leading-snug p-2 rounded shadow-inner mb-1"
        spellcheck="false" rows="8"
        v-model="encoded.text"></textarea>
      <div class="flex items-center justify-between flex-wrap -m-1">
        <input type="password"
          placeholder="Secret"
          :disabled="decryption.isRunning"
          class="border shadow-inner rounded px-2 min-w-0 m-1 flex-grow max-w-xs"
          v-model="encoded.password" />
        <button class="rounded px-2 text-white m-1"
          :disabled="!decryptBtn"
          :class="decryptBtn ? 'bg-gray-700' : 'bg-gray-400'"
          @click.prevent="decrypt"
          >Open</button>
      </div>
      <div v-if="decryption.error"
        class="bg-red-200 rounded text-red-700 px-2 mt-2 inline-block">{{ decryption.error }}</div>
    </form>
  </page-content>
  <div class="max-w-xl w-full">
    <div v-if="encodedAt"
      class="mb-4 text-sm italic text-gray-600 px-4 sm:px-6">{{ encodedAt }}</div>
    <pre v-if="doc"
      class="shadow p-4 rounded mb-4"
      v-html="doc" />
  </div>
</template>

<script lang="ts">
import { computed, defineComponent, reactive, watch, ref } from 'vue'
import Navbar from './Navbar.vue'
import PageContent from './PageContent.vue'
import { thread } from '@/worker/interface'
import { mtsdCreationDate, MtsdDocument } from '@/worker/native'
import { toBocr16, fromBocr16 } from '@/util/bocr16'
import { globalState } from '@/util/global'

export default defineComponent({
  components: { Navbar, PageContent },
  setup () {
    const encoded = reactive({
      text: '',
      password: '',
      isValid: undefined as boolean | undefined
    })

    if (globalState.encrypted) {
      encoded.text = toBocr16(globalState.encrypted)
    }

    const textBin = computed(() => {
      const text = encoded.text.replaceAll(/\s+/g, '')
      if (!text.length) return null

      try {
        return fromBocr16(text)
      } catch (e) {
        return null
      }
    })

    watch(textBin, async () => {
      if (!textBin.value) {
        encoded.isValid = !encoded.text.trim().length ? undefined : false
        return
      }
      if (encoded.isValid) {
        encoded.isValid = undefined
      }
      encoded.isValid = await thread.mtsdIsValid(textBin.value)
    }, { immediate: true })

    const encodedAt = computed(() => {
      if (!encoded.isValid || !textBin.value) {
        return undefined
      }
      return mtsdCreationDate(textBin.value).toLocaleString(undefined, { year: 'numeric', month: 'short', day: '2-digit', hour: '2-digit', minute: '2-digit', timeZoneName: 'short' })
    })

    const decryption = reactive({
      isRunning: false,
      error: ''
    })
    const doc = ref(null as null | string)
    const decryptBtn = computed(() => encoded.isValid && !decryption.isRunning)
    async function decrypt () {
      try {
        decryption.isRunning = true
        decryption.error = ''
        const pass = encoded.password
        encoded.password = ''
        const doc_ = await thread.mtsdDecrypt(textBin.value!, pass)
        doc.value = doc_.map(record =>
          record.map(field => {
            const isMultiline = field.value.includes('\n')
            return isMultiline
              ? `${field.key}:\n${field.value.split('\n').map(ln => `  ${ln}`).join('\n')}`
              : `${field.key}: ${field.value}`
          }).join('\n')
        ).join('\n---\n')
      } catch (e) {
        decryption.error = (e as Error).message
      } finally {
        decryption.isRunning = false
      }
    }

    async function fromFile (e: Event) {
      const file = (e.target as HTMLInputElement)!.files![0]
      const content = new Uint8Array(await file.arrayBuffer())
      encoded.text = toBocr16(content)
      globalState.encrypted = content
    }

    return {
      encoded,
      encodedAt,
      decryptBtn,
      decrypt,
      decryption,
      doc,
      fromFile
    }
  }
})
</script>

<style lang="postcss">
</style>
