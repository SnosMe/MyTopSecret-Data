<template>
  <router-view />
  <footer class="text-xs mb-4 whitespace-no-wrap">
    <span>&copy; {{ new Date().getFullYear() }} Alexander Drozdov</span>
  </footer>
</template>

<script lang="ts">
import { defineComponent } from 'vue'

export default defineComponent({
  name: 'App',
  components: {}
})
</script>

<style lang="postcss">
@tailwind base;
@tailwind utilities;

@font-face {
  font-family: 'Inconsolata SemiExpandedSemiBold';
  font-weight: normal;
  font-style: normal;
  src: url('./assets/fonts/InconsolataSemiExpanded-SemiBold.ttf') format('truetype')
}

.font-ocr {
  font-family: "Inconsolata SemiExpandedSemiBold", Menlo, Monaco, Consolas, "Liberation Mono", "Courier New", monospace;
}

#app {
  display: flex;
  flex-direction: column;
  align-items: center;
  min-height: 100vh;
}

[disabled] {
  @apply cursor-not-allowed;
}
</style>
