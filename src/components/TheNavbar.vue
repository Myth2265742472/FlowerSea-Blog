<script setup>
import { onMounted, onUnmounted, ref } from 'vue'
import { useRoute } from 'vue-router'

const scrolled = ref(false)
const mobileMenuOpen = ref(false)
const route = useRoute()

const onScroll = () => { scrolled.value = window.scrollY > 40 }
onMounted(() => window.addEventListener('scroll', onScroll))
onUnmounted(() => window.removeEventListener('scroll', onScroll))

const closeMenu = () => { mobileMenuOpen.value = false }
</script>

<template>
  <nav class="navbar" :class="{ scrolled }">
    <div class="nav-container">
      <RouterLink to="/" class="logo" @click="closeMenu">
        <span class="logo-bracket">&lt;</span>FlowerSea<span class="logo-slash">/</span><span class="logo-bracket">&gt;</span>
      </RouterLink>
      <ul class="nav-links" :class="{ open: mobileMenuOpen }">
        <li><RouterLink to="/" :class="{ active: route.name === 'home' }" @click="closeMenu">首页</RouterLink></li>
        <li><RouterLink to="/archive" :class="{ active: route.name === 'archive' }" @click="closeMenu">归档</RouterLink></li>
        <li><RouterLink to="/about" :class="{ active: route.name === 'about' }" @click="closeMenu">关于</RouterLink></li>
        <li><a href="https://github.com/Myth2265742472" target="_blank" rel="noopener" class="nav-cta" @click="closeMenu">GitHub</a></li>
      </ul>
      <button class="menu-toggle" @click="mobileMenuOpen = !mobileMenuOpen" aria-label="菜单">
        <span></span><span></span><span></span>
      </button>
    </div>
  </nav>
</template>
