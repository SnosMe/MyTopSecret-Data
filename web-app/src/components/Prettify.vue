<template>
  <div class="max-w-xl w-full">
    <div class="p-4 sm:p-6 bg-gray-100 shadow sm:shadow-none sm:rounded sm:border mb-4">
      <div class="hidden sm:flex mb-4">
        <div class="w-1/2 flex">
          <label for="bocr16_chars" class="whitespace-no-wrap mr-4">Characters / block:</label>
          <input v-model.number="charsPerBlock" type="number" id="bocr16_chars" min="1" :class="$style.numberInput">
        </div>
        <div class="w-1/2 ml-8 flex">
          <label for="bocr16_blocks" class="whitespace-no-wrap mr-4">Blocks / line:</label>
          <input v-model.number="blocksPerLine" type="number" id="bocr16_blocks" min="1" :class="$style.numberInput">
        </div>
      </div>
      <div class="flex justify-between mb-2 items-baseline">
        <button class="rounded px-2 text-white bg-gray-700"
          @click="saveMtsd">Download _.mtsd.bin</button>
        <a href="/fonts/InconsolataSemiExpanded-SemiBold.ttf"
          class="text-sm text-gray-700 hidden sm:block">Download font</a>
      </div>
      <pre v-if="encryptedText"
        :class="$style.encryptedText" class="font-ocr"
        v-html="encryptedText" />
      <div class="mt-4 sm:flex">
        <div class="mb-4 sm:mb-0 flex-1">
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
    </div>
    <img v-if="dmtxImg"
      :class="$style.dmtxImg"
      :src="dmtxImg" />
  </div>
</template>

<script lang="ts">
import { defineComponent, shallowRef, watchEffect, computed } from 'vue'
import { thread } from '@/worker/interface'
import { toBocr16 } from '@/util/bocr16'
import { saveFileAs } from '@/util/saveFileAs'

export default defineComponent({
  setup () {
    const encrypted = shallowRef(
      new Uint8Array([32, 33, 34, 35, 36, 37, 0, 76, 123, 255, 114])
    )
    const charsPerBlock = shallowRef(5)
    const blocksPerLine = shallowRef(4)

    const encryptedText = computed(() => {
      // eslint-disable-next-line @typescript-eslint/no-non-null-assertion
      return toBocr16(encrypted.value)
        .match(new RegExp(`.{1,${charsPerBlock.value}}`, 'g'))!.join(' ')
        .match(new RegExp(`.{1,${(charsPerBlock.value + 1) * blocksPerLine.value}}`, 'g'))!
        .map(_ => _.trim())
        .join('\n')
    })

    function saveMtsd () {
      saveFileAs(encrypted.value, '_.mtsd.bin', 'application/octet-stream')
    }

    // --- Data Matrix ---

    const dmtxImg = shallowRef<string | null>(null)
    const moduleSize = shallowRef(5)
    const marginSize = shallowRef(2)

    const canvas = document.createElement('canvas')
    const canvasCtx = canvas.getContext('2d', { alpha: false })
    if (!canvasCtx) throw new Error('Browser is not supported (canvas 2d)')

    watchEffect(async () => {
      const img = await thread.dmtxCreate(encrypted.value, moduleSize.value, moduleSize.value * marginSize.value)
      canvas.width = img.width
      canvas.height = img.height
      canvasCtx.putImageData(img, 0, 0)
      dmtxImg.value = canvas.toDataURL('image/png')
    })

    return {
      dmtxImg,
      moduleSize,
      marginSize,
      encryptedText,
      charsPerBlock,
      blocksPerLine,
      saveMtsd
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
