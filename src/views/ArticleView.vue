<script setup>
import { ref, onMounted, onUnmounted, watch, nextTick, computed } from 'vue'
import { useRoute } from 'vue-router'
import { loadArticle, generateTOC, enhanceCodeBlocks, getArticleList } from '../composables/useArticles'

const route = useRoute()
const article = ref(null)
const toc = ref([])
const activeToc = ref('')
const contentRef = ref(null)

const articleList = getArticleList()

// 上一篇 / 下一篇（列表是降序，最新在前）
const navPrev = computed(() => {
  const idx = articleList.findIndex((a) => a.id === route.params.id)
  return idx > 0 ? articleList[idx - 1] : null
})
const navNext = computed(() => {
  const idx = articleList.findIndex((a) => a.id === route.params.id)
  return idx >= 0 && idx < articleList.length - 1 ? articleList[idx + 1] : null
})

async function load(id) {
  article.value = null
  activeToc.value = ''
  const data = await loadArticle(id)
  if (!data) return
  article.value = data
  await nextTick()
  // 代码高亮
  if (contentRef.value) enhanceCodeBlocks(contentRef.value)
  // 生成 TOC
  toc.value = generateTOC(data.rawMarkdown)
  // 给 h2/h3 加锚点 id，便于 TOC 跳转
  if (contentRef.value) {
    contentRef.value.querySelectorAll('h2, h3').forEach((h) => {
      const text = h.textContent
        .replace(/\*\*(.+?)\*\*/g, '$1')
        .replace(/\*(.+?)\*/g, '$1')
        .trim()
      h.id = 'toc-' + text.replace(/[^\w\u4e00-\u9fff]+/g, '-').replace(/^-+|-+$/g, '').toLowerCase()
    })
  }
}

// 点击 TOC 项：平滑滚动到对应标题
function scrollToHeading(id) {
  activeToc.value = id
  const el = document.getElementById(id)
  if (el) el.scrollIntoView({ behavior: 'smooth', block: 'start' })
}

// 滚动高亮 TOC
function onScroll() {
  if (!toc.value.length || !contentRef.value) return
  const headings = contentRef.value.querySelectorAll('h2[id], h3[id]')
  let current = ''
  for (const h of headings) {
    const rect = h.getBoundingClientRect()
    if (rect.top <= 120) current = h.id
  }
  activeToc.value = current
}

onMounted(() => {
  load(route.params.id)
  window.addEventListener('scroll', onScroll, { passive: true })
})
onUnmounted(() => {
  window.removeEventListener('scroll', onScroll)
})

// 路由参数变化时重新加载
watch(() => route.params.id, (id) => {
  if (id) load(id)
})
</script>

<template>
  <article class="article-detail">
    <div v-if="!article" class="loading">加载中...</div>

    <template v-else>
      <header class="article-header">
        <div class="article-category"><span class="category-dot"></span>{{ article.meta.category }}</div>
        <h1 class="article-title">{{ article.meta.title }}</h1>
        <div class="article-info">
          <span>{{ article.meta.date }}</span>
          <span class="dot"></span>
          <span>{{ article.meta.author }}</span>
          <span class="dot"></span>
          <span>{{ article.meta.comments }}</span>
        </div>
      </header>

      <div class="article-content-wrapper">
        <div ref="contentRef" class="article-body prose" v-html="article.content"></div>

        <nav v-if="toc.length" class="article-toc" aria-label="文章目录">
          <div class="toc-title">目录</div>
          <ul class="toc-list">
            <li v-for="item in toc" :key="item.id" class="toc-item" :class="`toc-h${item.level}`">
              <a :href="`#${item.id}`" :class="{ 'toc-active': activeToc === item.id }" @click.prevent="scrollToHeading(item.id)">
                {{ item.text }}
              </a>
            </li>
          </ul>
        </nav>
      </div>

      <nav v-if="navPrev || navNext" class="article-nav" aria-label="文章导航">
        <div v-if="navNext" class="article-nav-item">
          <div class="nav-label">← 上一篇</div>
          <RouterLink :to="`/article/${navNext.id}`">{{ navNext.title }}</RouterLink>
        </div>
        <div v-else class="article-nav-item" style="visibility:hidden"></div>
        <div v-if="navPrev" class="article-nav-item" style="text-align:right">
          <div class="nav-label">下一篇 →</div>
          <RouterLink :to="`/article/${navPrev.id}`">{{ navPrev.title }}</RouterLink>
        </div>
      </nav>
    </template>
  </article>
</template>
