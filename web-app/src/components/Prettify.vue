<template>
  <navbar />
  <page-content>
    <div class="flex-wrap sm:flex-nowrap flex items-baseline">
      <div class="w-full sm:w-1/2 flex">
        <label for="bocr16_chars" class="whitespace-nowrap mr-4">Characters / block:</label>
        <input v-model.number="charsPerBlock" type="number" id="bocr16_chars" min="1" :class="$style.numberInput">
      </div>
      <div class="w-full sm:w-1/2 flex sm:ml-8 mb-4 mt-1 sm:mt-0">
        <label for="bocr16_blocks" class="whitespace-nowrap mr-4">Blocks / line:</label>
        <input v-model.number="blocksPerLine" type="number" id="bocr16_blocks" min="1" :class="$style.numberInput">
      </div>
    </div>
    <div class="flex mb-2 items-baseline">
      <button class="rounded px-2 text-white bg-gray-700"
        @click="saveMtsd">Download _.mtsd.bin</button>
      <a :href="`${publicPath}fonts/InconsolataSemiExpanded-SemiBold.ttf`" download
        class="text-sm text-gray-700 ml-2 hidden sm:block">Download font</a>
      <select v-model="binToTextKind" class="ml-auto border rounded ml-2 px-1">
        <option>hex</option>
        <option>base64</option>
        <option>bocr16</option>
      </select>
    </div>
    <pre v-if="encryptedText"
      :class="$style.encryptedText" class="font-ocr"
      v-html="encryptedText" />
    <div class="mt-4 sm:flex">
      <div class="flex-1 mb-4 sm:mb-0">
        <label for="dmtx_module_size">Module size: {{ moduleSize }} px</label>
        <div>
          <input v-model.number="moduleSize" type="range" id="dmtx_module_size" min="1" max="20" class="w-full">
          <div :class="$style.rangeInputBounds">
            <span>1 px</span><span>20 px</span>
          </div>
        </div>
      </div>
      <div class="flex-1 sm:ml-8">
        <label for="dmtx_margin">Margin size: {{ marginSize }} mod</label>
        <div>
          <input v-model.number="marginSize" type="range" id="dmtx_margin" min="0" max="5" class="w-full">
          <div :class="$style.rangeInputBounds">
            <span>0</span><span>5 modules</span>
          </div>
        </div>
      </div>
    </div>
  </page-content>
  <img v-if="dmtxImg"
    :class="$style.dmtxImg"
    :src="dmtxImg" />
</template>

<script lang="ts">
import { defineComponent, shallowRef, watch, computed } from 'vue'
import Navbar from './Navbar.vue'
import PageContent from './PageContent.vue'
import { thread } from '@/worker/interface'
import { toText } from '@/util/binText'
import { saveFileAs } from '@/util/saveFileAs'
import { globalState } from '@/util/global'

export default defineComponent({
  components: { Navbar, PageContent },
  beforeRouteEnter (to, from, next) {
    if (!globalState.encrypted) {
      next('/')
    } else {
      next(true)
    }
  },
  setup () {
    const charsPerBlock = shallowRef(5)
    const blocksPerLine = shallowRef(4)
    const binToTextKind = shallowRef('hex')

    const encryptedText = computed(() => {
      // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
      return toText(binToTextKind.value, globalState.encrypted!)
        .match(new RegExp(`.{1,${charsPerBlock.value}}`, 'g'))!.join(' ')
        .match(new RegExp(`.{1,${(charsPerBlock.value + 1) * blocksPerLine.value}}`, 'g'))!
        .map(_ => _.trim())
        .join('\n')
    })

    function saveMtsd () {
      saveFileAs(globalState.encrypted!, '_.mtsd.bin', 'application/octet-stream')
    }

    // --- Data Matrix ---

    const dmtxImg = shallowRef<string | null>(null)
    const moduleSize = shallowRef(5)
    const marginSize = shallowRef(2)

    const canvas = document.createElement('canvas')
    const canvasCtx = canvas.getContext('2d', { alpha: false })
    if (!canvasCtx) throw new Error('Browser is not supported (canvas 2d)')

    watch([moduleSize, marginSize], async () => {
      const img = await thread.dmtxCreate(globalState.encrypted!, moduleSize.value, moduleSize.value * marginSize.value)
      canvas.width = img.width
      canvas.height = img.height
      canvasCtx.putImageData(img, 0, 0)
      dmtxImg.value = canvas.toDataURL('image/png')
    }, { immediate: true })

    return {
      dmtxImg,
      moduleSize,
      marginSize,
      encryptedText,
      charsPerBlock,
      blocksPerLine,
      binToTextKind,
      saveMtsd,
      publicPath: process.env.BASE_URL
    }
  }
})
</script>

<style lang="postcss" module>
.dmtxImg {
  @apply mx-auto mb-4 shadow;
  image-rendering: pixelated;
}

.rangeInputBounds {
  @apply flex justify-between text-xs text-gray-700 leading-none;
}

.encryptedText {
  @apply p-4 bg-white rounded border overflow-auto text-sm;
  line-height: 1.15;
  letter-spacing: 0.03125em;
}

.numberInput {
  @apply min-w-0 w-full border rounded px-2;
}
</style>
