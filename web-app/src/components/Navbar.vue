<template>
  <div class="p-4 sm:p-6 max-w-xl border-b sm:border-none w-full">
    <slot name="primary">
      <div class="flex -m-1" id="nav">
        <router-link to="/decrypt" class="p-1">Decrypt</router-link>
        <router-link to="/encrypt" class="p-1 ml-1">Encrypt</router-link>
        <router-link to="/prettify" class="p-1 ml-1" v-if="showPrettify">Prettify</router-link>
      </div>
    </slot>
    <slot name="secondary" />
  </div>
</template>

<script lang="ts">
import { defineComponent, computed } from 'vue'
import { globalState } from '@/util/global'

export default defineComponent({
  setup () {
    return {
      showPrettify: computed(() => Boolean(globalState.encrypted))
    }
  }
})
</script>

<style lang="postcss">
#nav > .router-link-active {
  @apply font-semibold;
}
</style>
