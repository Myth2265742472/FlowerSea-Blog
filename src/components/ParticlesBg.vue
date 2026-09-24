<script setup>
import { onMounted, onUnmounted, ref } from 'vue'

const canvas = ref(null)
let ctx, raf, particles = []
const CONFIG = { count: 80, color: '99,102,241', linkColor: '99,102,241', maxDist: 140, speed: 0.35 }

onMounted(() => {
  const c = canvas.value
  ctx = c.getContext('2d')
  const resize = () => { c.width = window.innerWidth; c.height = window.innerHeight }
  resize()
  window.addEventListener('resize', resize)

  for (let i = 0; i < CONFIG.count; i++) {
    particles.push({
      x: Math.random() * c.width, y: Math.random() * c.height,
      vx: (Math.random() - 0.5) * CONFIG.speed,
      vy: (Math.random() - 0.5) * CONFIG.speed,
      r: Math.random() * 1.8 + 0.4
    })
  }

  const mouse = { x: -1000, y: -1000 }
  const onMove = (e) => { mouse.x = e.clientX; mouse.y = e.clientY }
  window.addEventListener('mousemove', onMove)

  const tick = () => {
    ctx.clearRect(0, 0, c.width, c.height)
    particles.forEach((p) => {
      p.x += p.vx; p.y += p.vy
      if (p.x < 0 || p.x > c.width) p.vx *= -1
      if (p.y < 0 || p.y > c.height) p.vy *= -1
      const dx = p.x - mouse.x, dy = p.y - mouse.y
      const d = Math.hypot(dx, dy)
      if (d < 120) { p.x += dx / d * 0.6; p.y += dy / d * 0.6 }
      ctx.beginPath()
      ctx.arc(p.x, p.y, p.r, 0, Math.PI * 2)
      ctx.fillStyle = `rgba(${CONFIG.color},0.6)`
      ctx.fill()
    })
    for (let i = 0; i < particles.length; i++) {
      for (let j = i + 1; j < particles.length; j++) {
        const a = particles[i], b = particles[j]
        const d = Math.hypot(a.x - b.x, a.y - b.y)
        if (d < CONFIG.maxDist) {
          ctx.beginPath()
          ctx.moveTo(a.x, a.y); ctx.lineTo(b.x, b.y)
          ctx.strokeStyle = `rgba(${CONFIG.linkColor},${(1 - d / CONFIG.maxDist) * 0.15})`
          ctx.lineWidth = 0.6
          ctx.stroke()
        }
      }
    }
    raf = requestAnimationFrame(tick)
  }
  tick()
  window._cleanupParticles = () => { cancelAnimationFrame(raf); window.removeEventListener('resize', resize); window.removeEventListener('mousemove', onMove) }
})

onUnmounted(() => { if (window._cleanupParticles) window._cleanupParticles() })
</script>

<template>
  <canvas ref="canvas" id="particles"></canvas>
</template>
