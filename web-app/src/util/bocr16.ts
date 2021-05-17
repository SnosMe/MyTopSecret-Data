
const ALPHABET = [
  '2', '3', 'a', 'b', 'c', 'd', 'e', 'f',
  'k', 'n', 'p', 't', 'u', 'v', 'x', 'y'
]

const BYTE_TO_BOCR16 = new Array(256).fill(undefined).map((_, byte) => {
  return ALPHABET[byte >> 4] + ALPHABET[byte % 16]
})

const BOCR16_TO_BYTE = new Map(
  BYTE_TO_BOCR16.map((str, byte) => [str, byte])
)

export function toBocr16 (data: Uint8Array): string {
  return Array.from(data)
    .map(byte => BYTE_TO_BOCR16[byte])
    .join('')
}

export function fromBocr16 (text: string): Uint8Array {
  if (text.length % 2 !== 0) {
    throw new Error('Invalid bocr16 text')
  }

  const size = text.length / 2
  const bytes = new Uint8Array(size)
  for (let i = 0; i < size; ++i) {
    const byte = BOCR16_TO_BYTE.get(text.substr(i * 2, 2))
    if (byte === undefined) {
      throw new Error('Invalid bocr16 text')
    }
    bytes[i] = byte
  }

  return bytes
}
