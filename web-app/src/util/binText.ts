import { codec, base64url } from 'rfc4648'

export function toText (kind: string, data: Uint8Array): string {
  if (kind === 'hex') {
    return codec.stringify(data, {
      chars: '0123456789abcdef',
      bits: 4
    })
  } else if (kind === 'base64') {
    return base64url.stringify(data, { pad: false })
  } else if (kind === 'bocr16') {
    return codec.stringify(data, {
      chars: '23abcdefknptuvxy',
      bits: 4
    })
  } else {
    throw new Error('Not implemented')
  }
}

export function toBin (kind: string, text: string): Uint8Array {
  if (kind === 'hex') {
    return codec.parse(text.toLowerCase(), {
      chars: '0123456789abcdef',
      bits: 4
    })
  } else if (kind === 'base64') {
    return base64url.parse(text, { loose: true })
  } else if (kind === 'bocr16') {
    return codec.parse(text, {
      chars: '23abcdefknptuvxy',
      bits: 4
    })
  } else {
    throw new Error('Not implemented')
  }
}
