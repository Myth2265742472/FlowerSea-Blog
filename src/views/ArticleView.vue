<script setup>
import { ref, onMounted, onUnmounted, nextTick, computed } from 'vue'
import { useRoute, useRouter, onBeforeRouteUpdate } from 'vue-router'
import { loadArticle, enhanceCodeBlocks, generateTOC, getArticleList } from '../composables/useArticles'

const route = useRoute()
const router = useRouter()
const allArticles = getArticleList()

const article = ref(null)
const toc = ref([])
const activeHeading = ref('')
const contentRef = ref(null)
let observer

const load = async (id) => {
  article.value = null
  const data = await loadArticle(id)
  if (!data) { router.push('/'); return }
  article.value = data
  toc.value = generateTOC(data.rawMarkdown)
  await nextTick()
  if (contentRef.value) enhanceCodeBlocks(contentRef.value)
  setupObserver()
  window.scrollTo(0, 0)
}

const setupObserver = () => {
  if (observer) observer.disconnect()
  if (!toc.value.length || !contentRef.value) return
  const ids = toc.value.map((h) => h.id)
  const els = ids.map((id) => document.getElementById(id)).filter(Boolean)
  if (!els.length) return
  observer = new IntersectionObserver((entries) => {
    entries.forEach((e) => {
      if (e.isIntersecting) activeHeading.value = e.target.id
    })
  }, { rootMargin: '-80px 0px -70% 0px' })
  els.forEach((el) => observer.observe(el))
}

const onScroll = () => {
  const bar = document.getElementById('reading-progress')
  if (!bar) return
  const sh = document.documentElement.scrollHeight - window.innerHeight
  bar.style.width = (sh > 0 ? (window.scrollY / sh) * 100 : 0) + '%'
}

// 相邻文章
const neighbors = computed(() => {
  if (!article.value) return { prev: null, next: null }
  const idx = allArticles.findIndex((a) => a.id === article.value.id)
  return {
    next: idx > 0 ? allArticles[idx - 1] : null, // 列表降序，idx-1 是更新的
    prev: idx < allArticles.length - 1 ? allArticles[idx + 1] : null
  }
})

onMounted(() => { load(route.params.id); window.addEventListener('scroll', onScroll) })
onBeforeRouteUpdate((to) => { if (to.params.id !== route.params.id) load(to.params.id) })
onUnmounted(() => { if (observer) observer.disconnect(); window.removeEventListener('scroll', onScroll) })
</script>

<template>
  <div class="article-detail">
    <div v-if="!article" class="loading">加载中</div>
    <template v-else>
      <a href="javascript:history.back()" class="back-link">
        <svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2"><path d="M19 12H5M12 19l-7-7 7-7" /></svg>
        返回
      </a>

      <header class="article-header">
        <h1 class="article-title">{{ article.meta.title }}</h1>
        <div class="article-info">
          <span>{{ article.meta.date }}</span>
          <span class="dot"></span>
          <span>{{ article.meta.author }}</span>
          <span class="dot"></span>
          <span>{{ article.meta.category }}</span>
          <span class="dot"></span>
          <span>{{ article.meta.comments }}</span>
        </div>
      </header>

      <div class="article-content-wrapper">
        <div ref="contentRef" class="article-body prose" v-html="article.content"></div>

        <aside v-if="toc.length" class="article-toc">
          <div class="toc-title">目录</div>
          <ul class="toc-list">
            <li v-for="h in toc" :key="h.id" class="toc-item" :class="{ 'toc-h3': h.level === 3 }">
              <a :href="`#${h.id}`" :class="{ 'toc-active': activeHeading === h.id }">{{ h.text }}</a>
            </li>
          </ul>
        </aside>
      </div>

      <div class="article-tags">
        <span class="tag-label">标签:</span>
        <a v-for="t in article.meta.tags" :key="t" href="#">#{{ t }}</a>
      </div>

      <nav class="article-nav" v-if="neighbors.prev || neighbors.next">
        <div v-if="neighbors.prev" class="article-nav-item" style="text-align:left">
          <div class="nav-label">← 上一篇</div>
          <RouterLink :to="`/article/${neighbors.prev.id}`">{{ neighbors.prev.title }}</RouterLink>
        </div>
        <div v-if="neighbors.next" class="article-nav-item" style="text-align:right">
          <div class="nav-label">下一篇 →</div>
          <RouterLink :to="`/article/${neighbors.next.id}`">{{ neighbors.next.title }}</RouterLink>
        </div>
      </nav>
    </template>
  </div>
</template>
