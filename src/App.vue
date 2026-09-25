<script setup>
import { useMobile } from './composables/useMobile'
import TheNavbar from './components/TheNavbar.vue'
import TheFooter from './components/TheFooter.vue'
import MusicPlayer from './components/MusicPlayer.vue'
import ParticlesBg from './components/ParticlesBg.vue'
import CursorEffect from './components/CursorEffect.vue'
import MobileTabbar from './components/MobileTabbar.vue'

// 初始化移动端检测（注册 onMounted 钩子）
useMobile()
</script>

<template>
  <ParticlesBg />
  <CursorEffect />
  <div class="reading-progress" id="reading-progress"></div>

  <TheNavbar />

  <main class="main-container">
    <RouterView />
  </main>

  <TheFooter />
  <MobileTabbar />
  <MusicPlayer />
</template>
