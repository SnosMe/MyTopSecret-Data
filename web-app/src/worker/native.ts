/// <reference types="emscripten" />

interface MtsdNative extends EmscriptenModule { /* eslint-disable camelcase */
  _b_mtsd_decrypt(encryptedDataPtr: number, encryptedSize: number,
                  passwordPtr: number, passLen: number): number

  _mtsd_container_is_valid(encryptedDataPtr: number, encryptedSize: number): number

  _b_dmtx_find_regions(imgDataPtr: number, imgW: number, imgH: number, budget: number): number

  _b_dmtx_create(dataPtr: number, dataSize: number,
                 modulePx: number, marginPx: number,
                 widthPtr: number, heightPtr: number): number

  _b_last_error(srcPtr: number, codePtr: number, msgPtr: number, msgLenPtr: number): void

  _b_mtsd_encrypt(textPtr: number, textSize: number,
                  passwordPtr: number, passLen: number): number

  stackSave(): number
  stackRestore(ptr: number): void
  stackAlloc(size: number): number
}

// eslint-disable-next-line @typescript-eslint/no-var-requires
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

export async function mtsdDecrypt (data: Uint8Array, password: string): Promise<MtsdDocument> {
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

export async function mtsdIsValid (data: Uint8Array): Promise<boolean> {
  const module = await load()

  const encryptedPtr = module._malloc(data.byteLength)
  module.HEAPU8.set(data, encryptedPtr)

  const isValid = module._mtsd_container_is_valid(encryptedPtr, data.byteLength)
  module._free(encryptedPtr)

  return Boolean(isValid)
}

export function mtsdCreationDate (data: Uint8Array): Date {
  const OFFSET = 3
  let ts = (data[OFFSET + 3] << 24) | (data[OFFSET + 2] << 16) | (data[OFFSET + 1] << 8) | data[OFFSET]

  const MTSD_DATE_FROM = 915148800
  ts += MTSD_DATE_FROM

  return new Date(ts * 1000)
}

export async function dmtxFindRegions (image: ImageData, budgetMs: number): Promise<DmtxRegion[]> {
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

export async function dmtxCreate (data: Uint8Array, modulePx: number, marginPx: number): Promise<ImageData> {
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

export async function mtsdEncrypt (text: string, password: string): Promise<Uint8Array> {
  const module = await load()

  const textUtf8 = TEXT_ENCODER.encode(text)
  const textPtr = module._malloc(textUtf8.byteLength)
  module.HEAPU8.set(textUtf8, textPtr)

  const passUtf8 = TEXT_ENCODER.encode(password)
  const passPtr = module._malloc(passUtf8.byteLength)
  module.HEAPU8.set(passUtf8, passPtr)

  try {
    const binPtr = module._b_mtsd_encrypt(textPtr, textUtf8.byteLength, passPtr, passUtf8.byteLength)
    if (binPtr) {
      const encryptedSize = module.HEAP32[binPtr / 4]
      const encrypted = new Uint8Array(module.HEAPU8.subarray(binPtr + 4, (binPtr + 4) + encryptedSize))
      module._free(binPtr)
      return encrypted
    } else {
      throw new Error('Cannot encrypt')
    }
  } finally {
    module._free(textPtr)
    module._free(passPtr)
  }
}

const ERROR_SOURCE = [
  'MTSD_ESELF',
  'MTSD_EARGON2',
  'MTSD_ELZMA',
  'MTSD_ERANDOMBYTES'
]

const MTSD_ERROR = [
  'MTSD_EMEMORY',
  'MTSD_EREADER',
  'MTSD_EENCODE_RECORD_SIZE',
  'MTSD_EENCODE_PAYLOAD_SIZE',
  'MTSD_EDECODE_CORRUPTED_PAYLOAD',
  'MTSD_EPARSE_UNKNOWN_KEY',
  'MTSD_ETIME'
]

interface MtsdErrror {
  source: string
  code: number | string
  message: string | null
}

export async function getLastError (): Promise<MtsdErrror> {
  const module = await load()

  let error: MtsdErrror

  const _stack = module.stackSave()
  {
    const srcPtr = module.stackAlloc(4)
    const codePtr = module.stackAlloc(4)
    const msgPtr = module.stackAlloc(4)
    const msgLenPtr = module.stackAlloc(4)

    module._b_last_error(srcPtr, codePtr, msgPtr, msgLenPtr)

    const src = module.HEAP32[srcPtr / 4]
    const code = module.HEAP32[codePtr / 4]
    const msgOffset = module.HEAP32[msgPtr / 4]
    const msgLen = module.HEAP32[msgLenPtr / 4]

    error = {
      source: ERROR_SOURCE[src],
      code: src === 0
        ? MTSD_ERROR[code]
        : code,
      message: msgLen
        ? TEXT_DECODER.decode(module.HEAPU8.subarray(msgOffset, msgOffset + msgLen))
        : null
    }
  }
  module.stackRestore(_stack)

  return error
}
