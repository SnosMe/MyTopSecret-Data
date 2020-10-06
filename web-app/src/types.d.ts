declare module '*.vue' {
  import { defineComponent } from 'vue'
  const component: ReturnType<typeof defineComponent>
  export default component
}

declare module 'worker-loader!*' {
  class WebpackWorker extends Worker {
    constructor()
  }

  export default WebpackWorker
}
