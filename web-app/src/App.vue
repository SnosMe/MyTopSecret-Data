<template>
  <div class="p-4 sm:p-6 mx-auto max-w-xl border-b sm:border-none">
    <div class="flex">
      <button class="font-semibold">Decode</button>
      <button class="ml-2">Encode</button>
    </div>
    <div class="flex mt-2">
      <button class="font-semibold">Text</button>
      <button class="ml-2">Scan</button>
      <button class="ml-2">File</button>
    </div>
  </div>
  <decode />
</template>

<script lang="ts">
import { defineComponent } from 'vue'
import Decode from './components/Decode.vue'

export default defineComponent({
  name: 'App',
  components: {
    Decode
  }
})
</script>

<style lang="postcss">
@tailwind base;
@tailwind utilities;

[disabled] {
  @apply cursor-not-allowed;
}
</style>
