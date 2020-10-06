<template>
  <div class="mx-auto max-w-xl">
    <form class="p-4 sm:p-6 bg-gray-100 shadow sm:shadow-none sm:rounded sm:border">
      <div class="flex items-baseline justify-between">
        <div class="flex items-center mb-2">
          <div class="flex items-center mr-4">
            <input type="radio" v-model="encoded.type" value="base64" id="type_base64">
            <label for="type_base64" class="ml-1">base64</label>
          </div>
          <div class="flex items-center">
            <input type="radio" v-model="encoded.type" value="hex" id="type_hex">
            <label for="type_hex" class="ml-1">hex</label>
          </div>
        </div>
        <div v-if="encoded.isValid === false"
          class="border bg-red-200 border-red-300 rounded text-red-700 px-2 leading-tight">Invalid</div>
        <div v-if="encoded.isValid === true"
          class="border bg-green-200 border-green-300 rounded text-green-700 px-2 leading-tight">Valid</div>
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
          >Decode</button>
      </div>
      <div v-if="decryption.error"
        class="bg-red-200 rounded text-red-700 px-2 mt-2 inline-block">{{ decryption.error }}</div>
    </form>
    <div v-if="encodedAt"
      class="mt-4 text-sm italic text-gray-600 px-4 sm:px-6">{{ encodedAt }}</div>
    <div v-if="doc" class="mt-4">
      <!-- eslint-disable-next-line vue/require-v-for-key -->
      <div v-for="record of doc"
        class="shadow p-4 rounded">
        <!-- eslint-disable-next-line vue/require-v-for-key -->
        <div v-for="field of record">
          <span class="text-blue-500 font-semibold">{{ field.key }}</span>: {{ field.value }}
        </div>
      </div>
    </div>
  </div>
</template>

<script lang="ts">
import { computed, defineComponent, reactive, watch, ref } from 'vue'
import { thread } from '@/worker/interface'
import { getCreationDate, MtsdDocument } from '@/worker/native'

export default defineComponent({
  setup () {
    const encoded = reactive({
      text: '7D 25 D2 A0 C8 EE 28 70\n29 F7 CD 00 23 9E BB 49\nDF A6',
      type: 'hex',
      password: '',
      isValid: undefined as boolean | undefined
    })

    const textBin = computed(() => {
      const text = encoded.text.replaceAll(/\s+/g, '')
      if (!text.length || text.length % 2 !== 0) {
        return null
      }
      return new Uint8Array(
        text.match(/.{1,2}/g)!.map(byte => parseInt(byte, 16))
      )
    })

    watch(textBin, async () => {
      if (!textBin.value) {
        encoded.isValid = !encoded.text.trim().length ? undefined : false
        return
      }
      if (encoded.isValid) {
        encoded.isValid = undefined
      }
      encoded.isValid = await thread.isValid(textBin.value)
    })

    const encodedAt = computed(() => {
      if (!encoded.isValid || !textBin.value) {
        return undefined
      }
      return getCreationDate(textBin.value).toLocaleString(undefined, { year: 'numeric', month: 'short', day: '2-digit', hour: '2-digit', minute: '2-digit', timeZoneName: 'short' })
    })

    const decryption = reactive({
      isRunning: false,
      error: ''
    })
    const doc = ref(null as null | MtsdDocument)
    const decryptBtn = computed(() => encoded.isValid && !decryption.isRunning)
    async function decrypt () {
      try {
        decryption.isRunning = true
        decryption.error = ''
        const pass = encoded.password
        encoded.password = ''
        doc.value = await thread.decrypt(textBin.value!, pass)
      } catch (e) {
        decryption.error = (e as Error).message
      } finally {
        decryption.isRunning = false
      }
    }

    return {
      encoded,
      encodedAt,
      decryptBtn,
      decrypt,
      decryption,
      doc
    }
  }
})
</script>

<style lang="postcss">
</style>
