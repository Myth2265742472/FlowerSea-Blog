<script setup>
import { onMounted, ref } from 'vue'
import { useMobile } from './composables/useMobile'
import TheNavbar from './components/TheNavbar.vue'
import TheFooter from './components/TheFooter.vue'
import MusicPlayer from './components/MusicPlayer.vue'
import ParticlesBg from './components/ParticlesBg.vue'
import CursorEffect from './components/CursorEffect.vue'
import MobileTabbar from './components/MobileTabbar.vue'

// 初始化移动端检测
useMobile()

const ready = ref(false)
onMounted(() => (ready.value = true))
</script>

<template>
  <ParticlesBg />
  <CursorEffect />
  <div class="reading-progress" id="reading-progress"></div>

  <TheNavbar />

  <main class="main-container">
    <RouterView v-slot="{ Component }">
      <Transition name="fade" mode="out-in">
        <component :is="Component" />
      </Transition>
    </RouterView>
  </main>

  <TheFooter />
  <MobileTabbar />
  <MusicPlayer />
</template>

<style>
.fade-enter-active, .fade-leave-active { transition: opacity 0.3s ease, transform 0.3s ease; }
.fade-enter-from { opacity: 0; transform: translateY(10px); }
.fade-leave-to { opacity: 0; transform: translateY(-10px); }
</style>
