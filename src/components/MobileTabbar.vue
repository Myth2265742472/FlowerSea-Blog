<script setup>
import { ref, watch } from 'vue'
import { useRoute, useRouter } from 'vue-router'
import { isMobile } from '../composables/useMobile'

const route = useRoute()
const router = useRouter()
const activeTab = ref(route.name || 'home')

watch(() => route.name, (n) => { activeTab.value = n || 'home' })

const tabs = [
  { name: 'home', label: '首页', icon: 'M3 12l9-9 9 9M5 10v10h14V10' },
  { name: 'archive', label: '归档', icon: 'M4 6h16M4 12h16M4 18h16' },
  { name: 'about', label: '关于', icon: 'M12 12m-1 1a1 1 0 11-2 0 1 1 0 012 0zM12 21a9 9 0 100-18 9 9 0 000 18z' },
  { name: 'github', label: 'GitHub', icon: 'M9 19c-5 1.5-5-2.5-7-3m14 6v-3.87a3.37 3.37 0 00-.94-2.61c3.14-.35 6.44-1.54 6.44-7A5.44 5.44 0 0020 4.77 5.07 5.07 0 0019.91 1S18.73.65 16 2.48a13.38 13.38 0 00-7 0C6.27.65 5.09 1 5.09 1A5.07 5.07 0 005 4.77a5.44 5.44 0 00-1.5 3.78c0 5.42 3.3 6.61 6.44 7A3.37 3.37 0 009 18.13V22' }
]

const go = (tab) => {
  if (tab.name === 'github') {
    window.open('https://github.com/Myth2265742472', '_blank')
    return
  }
  router.push({ name: tab.name })
}
</script>

<template>
  <nav v-if="isMobile" class="mobile-tabbar">
    <button v-for="tab in tabs" :key="tab.name"
      class="tab" :class="{ active: activeTab === tab.name }"
      @click="go(tab)" :aria-label="tab.label">
      <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" stroke-linecap="round" stroke-linejoin="round">
        <path :d="tab.icon" />
      </svg>
      <span>{{ tab.label }}</span>
    </button>
  </nav>
</template>
