<template>
  <div class="relative w-full flex-1 overflow-hidden" style="max-height: 100vh; max-width: 640px; max-height: 480px;">
    <video ref="video"></video>
    <div class="absolute origin-top-left"
      :style="{
        width: streamInfo.width + 'px',
        height: streamInfo.height + 'px',
        transform: `scale(${streamInfo.scale}) scaleY(-1)`
      }">
      <!-- eslint-disable-next-line vue/require-v-for-key -->
      <div v-for="region in dtmxRegions"
        style="width: 1px; height: 1px;"
        class="absolute origin-top-left bg-opacity-50"
        :class="region.isValid ? 'bg-green-500' : 'bg-red-500'"
        :style="`transform: matrix3d(${region.matrix})`"
      />
    </div>
    <div class="absolute right-0 top-0 p-4">
      <button class="px-4 py-1 rounded-full bg-gray-800 shadow text-white bg-opacity-50"
        :class="{ 'line-through': true }">auto</button>
    </div>
    <div class="absolute left-0 top-0 p-4 text-sm text-white">
      <div v-if="streamInfo.width">{{ streamInfo.width }} x {{ streamInfo.height }}</div>
      <div v-if="streamInfo.fps">{{ streamInfo.fps }} FPS</div>
    </div>
  </div>
</template>

<script lang="ts">
import { defineComponent, reactive, onMounted, shallowRef } from 'vue'
import { thread } from '@/worker/interface'
import { isValid } from '@/worker/native'
import * as Comlink from 'comlink'

export default defineComponent({
  setup () {
    const video = shallowRef<HTMLVideoElement | null>(null)
    const streamInfo = reactive({
      width: 0,
      height: 0,
      fps: 0,
      scale: 1
    })
    const dtmxRegions = shallowRef([] as Array<{
      matrix: string
      isValid: boolean
    }>)

    const canvas = document.createElement('canvas')
    const canvasCtx = canvas.getContext('2d', { alpha: false })!

    let isFrameUpdated = true
    let isProcessingFrame = false

    function frameUpdater () {
      isFrameUpdated = true
      requestAnimationFrame(frameUpdater)
      if (!isProcessingFrame) {
        processFrame()
      }
    }

    async function processFrame () {
      isProcessingFrame = true

      canvasCtx.drawImage(video.value!, 0, 0)
      const image = canvasCtx.getImageData(0, 0, canvas.width, canvas.height)

      const found = await thread.findDmtx(Comlink.transfer(image, [image.data.buffer]))

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

      if (isFrameUpdated) {
        processFrame()
      }
    }

    onMounted(async () => {
      const el = video.value!

      try {
        const stream = await navigator.mediaDevices.getUserMedia({
          video: {
            facingMode: { ideal: 'environment' }
          }
        })
        const track = stream.getVideoTracks()[0]
        const cap = track.getCapabilities()

        await track.applyConstraints({
          width: cap.width?.max,
          height: cap.height?.max
        })

        el.srcObject = stream
        await el.play()

        const settings = track.getSettings()
        streamInfo.width = el.videoWidth
        streamInfo.height = el.videoHeight
        streamInfo.fps = Math.round(settings.frameRate ?? 0)
        streamInfo.scale = el.clientWidth / streamInfo.width

        canvas.width = streamInfo.width
        canvas.height = streamInfo.height

        requestAnimationFrame(frameUpdater)
      } catch (e) {
        if ((e as DOMException).name === 'NotFoundError') {
          console.log('not found')
        } else {
          console.log(e)
        }
      }
    })

    return {
      video,
      streamInfo,
      dtmxRegions
    }
  }
})
</script>

<style lang="postcss">
</style>
