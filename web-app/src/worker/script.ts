import * as Comlink from 'comlink'
import { decrypt, isValid } from './native'

const WorkerBody = {
  decrypt (data: Uint8Array, password: string) {
    return decrypt(data, password)
  },
  isValid (data: Uint8Array) {
    return isValid(data)
  }
}

Comlink.expose(WorkerBody)

export type WorkerRPC = Comlink.Remote<typeof WorkerBody>
