import { createRouter, createWebHashHistory } from 'vue-router'

// GitHub Pages 是静态托管，必须用 hash 路由，否则直接访问子路径会 404
const routes = [
  { path: '/', name: 'home', component: () => import('../views/HomeView.vue'), meta: { title: "FlowerSea's Blog — 代码与创造" } },
  { path: '/article/:id', name: 'article', component: () => import('../views/ArticleView.vue'), meta: { title: '文章详情 — FlowerSea' } },
  { path: '/archive', name: 'archive', component: () => import('../views/ArchiveView.vue'), meta: { title: '归档 — FlowerSea' } },
  { path: '/about', name: 'about', component: () => import('../views/AboutView.vue'), meta: { title: '关于 — FlowerSea' } },
  { path: '/:pathMatch(.*)*', redirect: '/' }
]

const router = createRouter({
  history: createWebHashHistory(),
  routes,
  scrollBehavior(to, from, savedPosition) {
    if (savedPosition) return savedPosition
    if (to.hash) return { el: to.hash, behavior: 'smooth', top: 80 }
    return { top: 0 }
  }
})

router.afterEach((to) => {
  if (to.meta?.title) document.title = to.meta.title
})

export default router
