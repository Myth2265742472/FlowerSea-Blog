<script setup>
const year = new Date().getFullYear()
</script>

<template>
  <footer class="footer">
    <div class="footer-container">
      <div class="footer-top">
        <div class="footer-brand">
          <RouterLink to="/" class="logo">
            <span class="logo-bracket">&lt;</span>FlowerSea<span class="logo-slash">/</span><span class="logo-bracket">&gt;</span>
          </RouterLink>
          <p>代码与创造 — 一个记录技术探索与工程实践的个人博客</p>
        </div>
        <div class="footer-links">
          <div class="footer-col">
            <h4>导航</h4>
            <RouterLink to="/">首页</RouterLink>
            <RouterLink to="/archive">归档</RouterLink>
            <RouterLink to="/about">关于</RouterLink>
          </div>
          <div class="footer-col">
            <h4>分类</h4>
            <a href="#">技术教程</a>
            <a href="#">前端开发</a>
            <a href="#">嵌入式开发</a>
          </div>
          <div class="footer-col">
            <h4>关注</h4>
            <a href="https://github.com/Myth2265742472" target="_blank" rel="noopener">GitHub</a>
            <a href="mailto:FlowerSea@example.com">Email</a>
          </div>
        </div>
      </div>
      <div class="footer-bottom">
        <p class="footer-code"><span class="code-keyword">const</span> <span class="code-var">flowersea</span> = <span class="code-func">blog</span>(<span class="code-string">"{{ year }}"</span>);</p>
        <p>© {{ year }} FlowerSea's Blog · Made with <span class="heart">♥</span> · Powered by Vue 3</p>
      </div>
    </div>
  </footer>
</template>
