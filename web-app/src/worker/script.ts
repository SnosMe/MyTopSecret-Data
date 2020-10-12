import * as Comlink from 'comlink'
import { mtsdDecrypt, mtsdIsValid, dmtxFindRegions, DmtxRegion, dmtxCreate } from './native'

const WorkerBody = {
  mtsdDecrypt (data: Uint8Array, password: string) {
    return mtsdDecrypt(data, password)
  },
  mtsdIsValid (data: Uint8Array) {
    return mtsdIsValid(data)
  },
  async dmtxFindRegions (image: ImageData, budgetMs: number): Promise<DmtxRegion[]> {
    const res = await dmtxFindRegions(image, budgetMs)
    return Comlink.transfer(res, res.filter(region => region.data).map(region => region.data!.buffer))
  },
  async dmtxCreate (data: Uint8Array, modulePx: number, marginPx: number): Promise<ImageData> {
    const res = await dmtxCreate(data, modulePx, marginPx)
    return Comlink.transfer(res, [res.data.buffer])
  }
}

Comlink.expose(WorkerBody)

export type WorkerRPC = Comlink.Remote<typeof WorkerBody>
