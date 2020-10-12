/// <reference types="emscripten" />

interface MtsdNative extends EmscriptenModule { /* eslint-disable @typescript-eslint/camelcase */
  _b_mtsd_decrypt(encryptedDataPtr: number, encryptedSize: number,
                  passwordPtr: number, passLen: number): number

  _mtsd_container_is_valid(encryptedDataPtr: number, encryptedSize: number): number

  _b_dmtx_find_regions(imgDataPtr: number, imgW: number, imgH: number, budget: number): number

  _b_dmtx_create(dataPtr: number, dataSize: number,
                 modulePx: number, marginPx: number,
                 widthPtr: number, heightPtr: number): number

  stackSave(): number
  stackRestore(ptr: number): void
  stackAlloc(size: number): number
}

const Module = require('./mtsd-native.js') as EmscriptenModuleFactory<MtsdNative>

let module_: ReturnType<typeof Module>

function load () {
  if (!module_) {
    module_ = Module()
  }
  return module_
}

const TEXT_DECODER = new TextDecoder('utf-8')
const TEXT_ENCODER = new TextEncoder()

const KEYID = [
  '__NULL',
  'email',
  'password',
  'login'
]

interface MtsdField {
  key: string
  value: string
}
type MtsdRecord = MtsdField[]
export type MtsdDocument = MtsdRecord[]

function unpackMtsdDocument (packed: Uint8Array): MtsdDocument {
  let offset = 0
  const reader = new DataView(packed.buffer, packed.byteOffset, packed.byteLength)

  const recordsCount = reader.getInt32(offset, true)
  offset += 4
  return new Array(recordsCount).fill(undefined).map<MtsdRecord>(() => {
    const fieldsCount = reader.getInt32(offset, true)
    offset += 4
    return new Array(fieldsCount).fill(undefined).map<MtsdField>(() => {
      const keyid = reader.getUint8(offset)
      offset += 1
      const valueLength = reader.getInt32(offset, true)
      offset += 4
      const value = TEXT_DECODER.decode(packed.subarray(offset, offset + valueLength))
      offset += valueLength
      return {
        key: KEYID[keyid],
        value: value
      }
    })
  })
}

export interface DmtxRegion {
  posMatrix: number[]
  data: Uint8Array | null
}

function unpackDmtxRegions (packed: Uint8Array): DmtxRegion[] {
  let offset = 0
  const reader = new DataView(packed.buffer, packed.byteOffset, packed.byteLength)

  const regionsCount = reader.getInt32(offset, true)
  offset += 4
  return new Array(regionsCount).fill(undefined).map<DmtxRegion>(() => {
    const posMatrix = new Array(9).fill(undefined).map<number>((_, idx) =>
      reader.getFloat64(offset + idx * 8, true)
    )
    offset += posMatrix.length * 8

    const dataLength = reader.getInt32(offset, true)
    offset += 4
    let data: DmtxRegion['data'] = null
    if (dataLength) {
      data = new Uint8Array(dataLength)
      data.set(packed.subarray(offset, offset + dataLength))
      offset += dataLength
    }

    return {
      posMatrix,
      data
    }
  })
}

export async function mtsdDecrypt (data: Uint8Array, password: string) {
  const module = await load()

  const encryptedPtr = module._malloc(data.byteLength)
  module.HEAPU8.set(data, encryptedPtr)

  const passUtf8 = TEXT_ENCODER.encode(password)
  const passPtr = module._malloc(passUtf8.byteLength)
  module.HEAPU8.set(passUtf8, passPtr)

  try {
    const binPtr = module._b_mtsd_decrypt(encryptedPtr, data.byteLength, passPtr, passUtf8.byteLength)
    if (binPtr) {
      const doc = unpackMtsdDocument(module.HEAPU8.subarray(binPtr))
      module._free(binPtr)
      return doc
    } else {
      throw new Error('Invalid password') // TODO: handle lowmem error
    }
  } finally {
    module._free(encryptedPtr)
    module._free(passPtr)
  }
}

export async function mtsdIsValid (data: Uint8Array) {
  const module = await load()

  const encryptedPtr = module._malloc(data.byteLength)
  module.HEAPU8.set(data, encryptedPtr)

  const isValid = module._mtsd_container_is_valid(encryptedPtr, data.byteLength)
  module._free(encryptedPtr)

  return Boolean(isValid)
}

export function mtsdCreationDate (data: Uint8Array) {
  const OFFSET = 3
  let ts = (data[OFFSET + 3] << 24) | (data[OFFSET + 2] << 16) | (data[OFFSET + 1] << 8) | data[OFFSET]

  const MTSD_DATE_FROM = 915148800
  ts += MTSD_DATE_FROM

  return new Date(ts * 1000)
}

export async function dmtxFindRegions (image: ImageData, budgetMs: number) {
  const module = await load()

  const imgDataPtr = module._malloc(image.data.byteLength)
  module.HEAPU8.set(image.data, imgDataPtr)

  const resPtr = module._b_dmtx_find_regions(imgDataPtr, image.width, image.height, budgetMs)
  module._free(imgDataPtr)

  if (resPtr) {
    const res = unpackDmtxRegions(module.HEAPU8.subarray(resPtr))
    module._free(resPtr)
    return res
  } else {
    return []
  }
}

export async function dmtxCreate (data: Uint8Array, modulePx: number, marginPx: number) {
  const module = await load()

  const _stack = module.stackSave()
  const widthPtr = module.stackAlloc(4)
  const heightPtr = module.stackAlloc(4)

  const dataPtr = module._malloc(data.byteLength)
  module.HEAPU8.set(data, dataPtr)

  const imgPtr = module._b_dmtx_create(dataPtr, data.byteLength, modulePx, marginPx, widthPtr, heightPtr)
  module._free(dataPtr)

  if (!imgPtr) {
    module.stackRestore(_stack)
    throw new Error('Cannot create Data Matrix')
  }

  const width = module.HEAP32[widthPtr / 4]
  const height = module.HEAP32[heightPtr / 4]
  module.stackRestore(_stack)

  const imgArray = new Uint8ClampedArray(
    module.HEAPU8.subarray(imgPtr, imgPtr + (4 * width * height))
  )
  module._free(imgPtr)

  return new ImageData(imgArray, width, height)
}
