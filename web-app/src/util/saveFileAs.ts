
export function saveFileAs (data: Uint8Array, fileName: string, mimeType: string): void {
  const blob = new Blob([data], { type: mimeType })
  const url = window.URL.createObjectURL(blob)

  const a = document.createElement('a')
  a.href = url
  a.rel = 'noopener'
  a.download = fileName
  a.click()

  setTimeout(() => {
    return window.URL.revokeObjectURL(url)
  }, 40 * 1000)
}
