/// <reference types="emscripten" />

interface MtsdNative extends EmscriptenModule { /* eslint-disable @typescript-eslint/camelcase */
  _decrypt(encryptedDataPtr: number, encryptedSize: number,
           passwordPtr: number, passLen: number): number

  _mtsd_container_is_valid(encryptedDataPtr: number, encryptedSize: number): number
}

const Module = require('./mtsd-native.js') as EmscriptenModuleFactory<MtsdNative>

let module_ = null! as ReturnType<typeof Module>

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

function decodeDocument (encoded: Uint8Array): MtsdDocument {
  let offset = 0
  const reader = new DataView(encoded.buffer, encoded.byteOffset, encoded.byteLength)

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
      const value = TEXT_DECODER.decode(encoded.subarray(offset, offset + valueLength))
      offset += valueLength
      return {
        key: KEYID[keyid],
        value: value
      }
    })
  })
}

export async function decrypt (data: Uint8Array, password: string) {
  const module = await load()

  const encryptedPtr = module._malloc(data.byteLength)
  module.HEAPU8.set(data, encryptedPtr)

  const passUtf8 = TEXT_ENCODER.encode(password)
  const passPtr = module._malloc(passUtf8.byteLength)
  module.HEAPU8.set(passUtf8, passPtr)

  try {
    const binPtr = module._decrypt(encryptedPtr, data.byteLength, passPtr, passUtf8.byteLength)
    if (binPtr) {
      const doc = decodeDocument(module.HEAPU8.subarray(binPtr))
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

export async function isValid (data: Uint8Array) {
  const module = await load()

  const encryptedPtr = module._malloc(data.byteLength)
  module.HEAPU8.set(data, encryptedPtr)

  const isValid = module._mtsd_container_is_valid(encryptedPtr, data.byteLength)
  module._free(encryptedPtr)

  return Boolean(isValid)
}

export function getCreationDate (data: Uint8Array) {
  const OFFSET = 3
  let ts = (data[OFFSET + 3] << 24) | (data[OFFSET + 2] << 16) | (data[OFFSET + 1] << 8) | data[OFFSET]

  const MTSD_DATE_FROM = 915148800
  ts += MTSD_DATE_FROM

  return new Date(ts * 1000)
}
