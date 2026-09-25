import { onMounted, ref } from 'vue'

// 移动端检测：UA 关键字 或 (窄屏≤768px + 触摸设备)
export function isMobileDevice() {
  if (typeof navigator === 'undefined') return false
  const ua = navigator.userAgent || ''
  const uaMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(ua)
  const narrowTouch = window.matchMedia('(max-width: 768px)').matches &&
    (navigator.maxTouchPoints > 0 || 'ontouchstart' in window)
  return uaMobile || narrowTouch
}

// 全局响应式移动端状态
const isMobile = ref(false)

export function useMobile() {
  onMounted(() => {
    isMobile.value = isMobileDevice()
    if (isMobile.value) {
      document.documentElement.classList.add('mobile-mode')
      document.body.classList.add('mobile-mode')
    }
  })
  return { isMobile }
}

export { isMobile }
