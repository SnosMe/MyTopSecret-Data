<template>
  <div class="p-4 sm:p-6 border-b sm:border-none w-full max-w-xl flex justify-between items-baseline">
    <router-link to="/decrypt">&laquo; Cancel</router-link>
    <div>Budget: <input v-model.number="finderBudget" type="number" class="w-16 text-center border rounded" min="4"> ms</div>
  </div>
  <div v-if="!streamInfo.error"
    class="relative flex-1 overflow-hidden mb-4"
    style="max-height: 100vh;"
  >
    <video ref="video" />
    <div class="absolute origin-top-left"
      :style="{
        width: streamInfo.width + 'px',
        height: streamInfo.height + 'px',
        transform: `scale(${streamInfo.scale}) scaleY(-1)`
      }">
      <!-- eslint-disable-next-line vue/require-v-for-key -->
      <div v-for="region in dtmxRegions"
        :class="[$style.region, region.isValid ? 'bg-green-500' : 'bg-red-500']"
        :style="`transform: matrix3d(${region.matrix})`"
      />
    </div>
    <div class="absolute right-0 top-0 p-4">
      <button class="px-4 py-1 rounded-full bg-gray-800 shadow text-white bg-opacity-50"
        @click="autoSelect = !autoSelect"
        :class="{ 'line-through': !autoSelect }">auto</button>
    </div>
    <div class="absolute left-0 top-0 p-4 text-sm text-white">
      <div v-if="streamInfo.width">{{ streamInfo.width }} x {{ streamInfo.height }}</div>
      <div v-if="streamInfo.fps">{{ streamInfo.fps }} FPS</div>
    </div>
  </div>
  <div v-if="streamInfo.error"
    class="bg-red-200 rounded text-red-700 px-2 m-4"
    >{{ streamInfo.error }}</div>
</template>

<script lang="ts">
import { defineComponent, reactive, onMounted, onBeforeUnmount, shallowRef } from 'vue'
import { thread } from '@/worker/interface'
import * as Comlink from 'comlink'
import { globalState } from '@/util/global'
import router from '@/router'

export default defineComponent({
  setup () {
    const video = shallowRef<HTMLVideoElement | null>(null)
    const streamInfo = reactive({
      width: 0,
      height: 0,
      fps: 0,
      scale: 1,
      error: ''
    })
    const dtmxRegions = shallowRef([] as Array<{
      matrix: string
      isValid: boolean
    }>)
    const finderBudget = shallowRef(41 /* 24 fps */)
    const autoSelect = shallowRef(true)

    let stream = null as MediaStream | null

    const canvas = document.createElement('canvas')
    const canvasCtx = canvas.getContext('2d', { alpha: false })!

    let rafId = 0
    let nextFrame = null as ImageData | null
    let isProcessingFrame = false

    function frameUpdater () {
      if (!nextFrame) {
        canvasCtx.drawImage(video.value!, 0, 0)
        nextFrame = canvasCtx.getImageData(0, 0, canvas.width, canvas.height)
      }

      rafId = requestAnimationFrame(frameUpdater)
      if (!isProcessingFrame) {
        processFrame()
      }
    }

    onBeforeUnmount(() => {
      cancelAnimationFrame(rafId)
      nextFrame = null

      if (stream) {
        stream.getTracks().forEach(track => {
          track.stop()
        })
      }
    })

    async function processFrame () {
      isProcessingFrame = true
      const image = nextFrame!
      nextFrame = null

      const found = await thread.dmtxFindRegions(
        Comlink.transfer(image, [image.data.buffer]),
        Number(finderBudget.value) || 4
      )

      if (autoSelect.value) {
        const firstValid = found.find(region => Boolean(region.data))
        if (firstValid) {
          globalState.encrypted = firstValid.data!
          await router.push('/decrypt')
          return
        }
      }

      dtmxRegions.value = found.map(region => {
        const mx = region.posMatrix
        return {
          matrix: (/* eslint-disable */
            `${mx[0]}, ${mx[1]},    0, ${mx[2]},` +
            `${mx[3]}, ${mx[4]},    0, ${mx[5]},` +
            `       0,        0,    1,        0,` +
            `${mx[6]}, ${mx[7]},    0, ${mx[8]} `
          ),
          isValid: Boolean(region.data)
        }
      })

      isProcessingFrame = false
      if (nextFrame) {
        processFrame()
      }
    }

    onMounted(async () => {
      const el = video.value!

      try {
        stream = await navigator.mediaDevices.getUserMedia({
          video: {
            facingMode: { ideal: 'environment' }
          }
        })
        const track = stream.getVideoTracks()[0]
        const cap = track.getCapabilities()

        // await track.applyConstraints({
        //   width: cap.width?.max,
        //   height: cap.height?.max
        // })

        el.srcObject = stream
        await el.play()

        const settings = track.getSettings()
        streamInfo.width = el.videoWidth
        streamInfo.height = el.videoHeight
        streamInfo.fps = Math.round(settings.frameRate ?? 0)
        streamInfo.scale = el.clientWidth / streamInfo.width

        canvas.width = streamInfo.width
        canvas.height = streamInfo.height

        rafId = requestAnimationFrame(frameUpdater)
      } catch (e) {
        if ((e as DOMException).name === 'NotFoundError') {
          streamInfo.error = 'No video devices found on your system'
        } else {
          streamInfo.error = `Unhandled error occurred (${ String(e) || 'n/a' })`
        }
      }
    })

    return {
      video,
      streamInfo,
      dtmxRegions,
      finderBudget,
      autoSelect
    }
  }
})
</script>

<style lang="postcss" module>
.region {
  @apply absolute origin-top-left bg-opacity-50;
  width: 1px;
  height: 1px;
}
</style>
