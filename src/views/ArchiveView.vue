<script setup>
import { computed } from 'vue'
import { getArticleList } from '../composables/useArticles'

const articles = getArticleList()
// 按月份分组
const grouped = computed(() => {
  const map = {}
  articles.forEach((a) => {
    const m = a.date.replace(/(\d+)年(\d+)月/, '$1年$2月')
    ;(map[m] = map[m] || []).push(a)
  })
  return Object.keys(map).sort().reverse().map((k) => ({ month: k, items: map[k] }))
})
</script>

<template>
  <div class="archive-page">
    <header class="section-header">
      <span class="section-tag">// archive</span>
      <h1 class="section-title">文章归档</h1>
    </header>

    <section v-for="g in grouped" :key="g.month" style="margin-bottom:48px;">
      <h2 class="archive-year">{{ g.month }}</h2>
      <div v-for="a in g.items" :key="a.id" class="archive-item">
        <RouterLink :to="`/article/${a.id}`">{{ a.title }}</RouterLink>
        <span class="date">{{ a.date }}</span>
      </div>
    </section>
  </div>
</template>
