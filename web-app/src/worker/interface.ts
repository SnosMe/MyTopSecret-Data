// eslint-disable-next-line import/no-webpack-loader-syntax
import Worker from 'worker-loader!./script'
import * as Comlink from 'comlink'
import type { WorkerRPC } from './script'

export const thread = Comlink.wrap<WorkerRPC>(new Worker())
