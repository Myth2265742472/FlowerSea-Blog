<script setup>
import { onMounted, onUnmounted, ref, computed } from 'vue'
import { getArticleList } from '../composables/useArticles'

const articles = getArticleList()
const total = articles.length

// 打字机
const typed = ref('')
const phrases = ['记录每一次思考', '探索代码的艺术', '分享工程实践', '构建有趣的东西']
let pIdx = 0, cIdx = 0, deleting = false, timer
const typeLoop = () => {
  const cur = phrases[pIdx]
  if (!deleting) {
    typed.value = cur.slice(0, cIdx + 1); cIdx++
    if (cIdx === cur.length) { deleting = true; timer = setTimeout(typeLoop, 1800); return }
  } else {
    typed.value = cur.slice(0, cIdx - 1); cIdx--
    if (cIdx === 0) { deleting = false; pIdx = (pIdx + 1) % phrases.length }
  }
  timer = setTimeout(typeLoop, deleting ? 50 : 90)
}

// 分页
const page = ref(1)
const perPage = ref(6)
const totalPages = computed(() => Math.ceil(articles.length / perPage.value))
const pagedArticles = computed(() => {
  const start = (page.value - 1) * perPage.value
  return articles.slice(start, start + perPage.value)
})

// 摘要展开
const expanded = ref({})
const toggleExcerpt = (id) => { expanded.value[id] = !expanded.value[id] }

// 滚动进度
const onScroll = () => {
  const bar = document.getElementById('reading-progress')
  if (!bar) return
  const sh = document.documentElement.scrollHeight - window.innerHeight
  bar.style.width = (sh > 0 ? (window.scrollY / sh) * 100 : 0) + '%'
}

onMounted(() => { typeLoop(); window.addEventListener('scroll', onScroll); onScroll() })
onUnmounted(() => { clearTimeout(timer); window.removeEventListener('scroll', onScroll) })

// 侧边栏数据
const recentPosts = articles.slice(0, 4)
const categories = [...new Set(articles.map((a) => a.category))]
const archives = [
  { label: '2026年6月', count: articles.filter((a) => a.date.includes('6月')).length },
  { label: '2026年3月', count: articles.filter((a) => a.date.includes('3月')).length }
]
</script>

<template>
  <section class="hero">
    <div class="hero-content">
      <div class="hero-tag"><span class="status-dot"></span> Available for new ideas</div>
      <h1 class="hero-title">
        <span class="line">代码与创造</span>
        <span class="line gradient-text">FlowerSea's Blog</span>
      </h1>
      <p class="hero-desc">{{ typed }}<span class="typed-cursor">▋</span></p>
      <div class="hero-actions">
        <RouterLink to="/archive" class="btn btn-primary">浏览文章 →</RouterLink>
        <RouterLink to="/about" class="btn btn-ghost">关于我</RouterLink>
      </div>
      <div class="hero-stats">
        <div class="stat"><span class="stat-number">{{ total }}</span><span class="stat-label">文章</span></div>
        <div class="stat-divider"></div>
        <div class="stat"><span class="stat-number">{{ categories.length }}</span><span class="stat-label">分类</span></div>
        <div class="stat-divider"></div>
        <div class="stat"><span class="stat-number">∞</span><span class="stat-label">探索</span></div>
      </div>
    </div>
  </section>

  <div class="main-layout">
    <main class="posts-section">
      <header class="section-header">
        <span class="section-tag">// latest_posts</span>
        <h2 class="section-title">最新文章</h2>
      </header>

      <div class="posts-grid">
        <article v-for="a in pagedArticles" :key="a.id" class="post-card">
          <div class="post-card-inner">
            <div class="post-category"><span class="category-dot"></span>{{ a.category }}</div>
            <h2><RouterLink :to="`/article/${a.id}`">{{ a.title }}</RouterLink></h2>
            <div class="article-meta-new">
              <span>{{ a.date }}</span>
              <span>{{ a.author }}</span>
              <span>{{ a.comments }}</span>
            </div>
            <div class="article-excerpt-wrapper">
              <span class="article-excerpt-toggle" :class="{ active: expanded[a.id] }" @click="toggleExcerpt(a.id)">
                <svg class="excerpt-arrow" viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2"><path d="M9 18l6-6-6-6" /></svg>
                {{ expanded[a.id] ? '收起摘要' : '展开摘要' }}
              </span>
              <div class="article-excerpt-content" :class="{ expanded: expanded[a.id] }">
                <p>{{ a.summary }}</p>
              </div>
            </div>
            <div class="post-footer-new">
              <RouterLink :to="`/article/${a.id}`" class="post-link-new">
                阅读全文
                <svg viewBox="0 0 24 24" width="14" height="14" fill="none" stroke="currentColor" stroke-width="2"><path d="M5 12h14M12 5l7 7-7 7" /></svg>
              </RouterLink>
              <div style="display:flex;gap:6px;flex-wrap:wrap;">
                <span v-for="t in a.tags.slice(0,2)" :key="t" style="font-size:11px;color:var(--text-muted);font-family:var(--font-mono);">#{{ t }}</span>
              </div>
            </div>
          </div>
        </article>
      </div>

      <nav v-if="totalPages > 1" class="pagination">
        <button class="page-btn" :disabled="page === 1" @click="page--">← 上一页</button>
        <span class="page-info">{{ page }} / {{ totalPages }}</span>
        <button class="page-btn" :disabled="page === totalPages" @click="page++">下一页 →</button>
      </nav>
    </main>

    <aside class="sidebar">
      <div class="widget">
        <h3 class="widget-title"><svg viewBox="0 0 24 24" width="16" height="16" fill="none" stroke="currentColor" stroke-width="2"><circle cx="11" cy="11" r="8" /><path d="M21 21l-4.35-4.35" /></svg>搜索</h3>
        <form class="search-form" @submit.prevent>
          <input type="text" placeholder="搜索文章..." aria-label="搜索" />
          <button type="submit">→</button>
        </form>
      </div>
      <div class="widget">
        <h3 class="widget-title">最新文章</h3>
        <ul>
          <li v-for="a in recentPosts" :key="a.id"><RouterLink :to="`/article/${a.id}`">{{ a.title }}</RouterLink></li>
        </ul>
      </div>
      <div class="widget">
        <h3 class="widget-title">分类</h3>
        <ul>
          <li v-for="c in categories" :key="c"><a href="#">{{ c }}</a></li>
        </ul>
      </div>
      <div class="widget">
        <h3 class="widget-title">归档</h3>
        <ul>
          <li v-for="ar in archives" :key="ar.label"><RouterLink to="/archive">{{ ar.label }} ({{ ar.count }})</RouterLink></li>
        </ul>
      </div>
    </aside>
  </div>
</template>
