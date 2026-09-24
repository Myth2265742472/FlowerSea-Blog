<script setup>
import { onMounted, onUnmounted, ref } from 'vue'

const dot = ref(null)
const ring = ref(null)
let dotX = 0, dotY = 0, ringX = 0, ringY = 0, targetX = 0, targetY = 0, raf

onMounted(() => {
  const onMove = (e) => { targetX = e.clientX; targetY = e.clientY }
  const onDown = () => ring.value?.classList.add('hover')
  const onUp = () => ring.value?.classList.remove('hover')
  document.addEventListener('mousemove', onMove)
  document.addEventListener('mousedown', onDown)
  document.addEventListener('mouseup', onUp)

  const hoverables = 'a, button, .post-card, input, .mobile-tabbar .tab, .article-nav-item'
  const addHover = () => { dot.value?.classList.add('hover'); ring.value?.classList.add('hover') }
  const removeHover = () => { dot.value?.classList.remove('hover'); ring.value?.classList.remove('hover') }
  document.addEventListener('mouseover', (e) => { if (e.target.closest(hoverables)) addHover() })
  document.addEventListener('mouseout', (e) => { if (e.target.closest(hoverables)) removeHover() })

  const animate = () => {
    dotX += (targetX - dotX) * 0.4
    dotY += (targetY - dotY) * 0.4
    ringX += (targetX - ringX) * 0.18
    ringY += (targetY - ringY) * 0.18
    if (dot.value) { dot.value.style.left = dotX + 'px'; dot.value.style.top = dotY + 'px' }
    if (ring.value) { ring.value.style.left = ringX + 'px'; ring.value.style.top = ringY + 'px' }
    raf = requestAnimationFrame(animate)
  }
  animate()
  window._cleanupCursor = () => {
    cancelAnimationFrame(raf)
    document.removeEventListener('mousemove', onMove)
    document.removeEventListener('mousedown', onDown)
    document.removeEventListener('mouseup', onUp)
  }
})

onUnmounted(() => { if (window._cleanupCursor) window._cleanupCursor() })
</script>

<template>
  <div ref="dot" class="cursor-dot"></div>
  <div ref="ring" class="cursor-ring"></div>
</template>
