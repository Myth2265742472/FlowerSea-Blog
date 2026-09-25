import { marked } from 'marked'
// 按需导入 highlight.js 核心 + 常用语言，避免全量包 900KB
import hljs from 'highlight.js/lib/core'
import javascript from 'highlight.js/lib/languages/javascript'
import typescript from 'highlight.js/lib/languages/typescript'
import python from 'highlight.js/lib/languages/python'
import bash from 'highlight.js/lib/languages/bash'
import c from 'highlight.js/lib/languages/c'
import cpp from 'highlight.js/lib/languages/cpp'
import json from 'highlight.js/lib/languages/json'
import xml from 'highlight.js/lib/languages/xml'
import markdown from 'highlight.js/lib/languages/markdown'
import css from 'highlight.js/lib/languages/css'
import ini from 'highlight.js/lib/languages/ini'
import plaintext from 'highlight.js/lib/languages/plaintext'
import 'highlight.js/styles/github-dark.css'

hljs.registerLanguage('javascript', javascript)
hljs.registerLanguage('js', javascript)
hljs.registerLanguage('typescript', typescript)
hljs.registerLanguage('ts', typescript)
hljs.registerLanguage('python', python)
hljs.registerLanguage('py', python)
hljs.registerLanguage('bash', bash)
hljs.registerLanguage('sh', bash)
hljs.registerLanguage('shell', bash)
hljs.registerLanguage('c', c)
hljs.registerLanguage('cpp', cpp)
hljs.registerLanguage('c++', cpp)
hljs.registerLanguage('json', json)
hljs.registerLanguage('html', xml)
hljs.registerLanguage('xml', xml)
hljs.registerLanguage('markdown', markdown)
hljs.registerLanguage('md', markdown)
hljs.registerLanguage('css', css)
hljs.registerLanguage('ini', ini)
hljs.registerLanguage('conf', ini)
hljs.registerLanguage('plaintext', plaintext)
hljs.registerLanguage('text', plaintext)

import { articlesMeta } from '../data/blog.js'

// marked v5+ 已移除 highlight 选项；这里仅解析，高亮在 DOM 渲染后补做
marked.setOptions({ gfm: true, breaks: false })

// 解析 Markdown → HTML
export function parseMarkdown(md) {
  if (!md) return ''
  return marked.parse(md)
}

// 对渲染后的容器补做代码高亮 + 语言标签
export function enhanceCodeBlocks(container) {
  if (!container || !hljs) return
  container.querySelectorAll('pre code').forEach((block) => {
    if (block.dataset.highlighted) return
    let lang = ''
    block.classList.forEach((c) => { if (c.indexOf('language-') === 0) lang = c.slice(9) })
    try { hljs.highlightElement(block) } catch (e) { /* 忽略单块异常 */ }
    block.dataset.highlighted = '1'
    const pre = block.parentElement
    if (pre && pre.tagName === 'PRE' && lang) pre.setAttribute('data-lang', lang)
  })
}

// 文章列表（降序，最新在前）
export function getArticleList() {
  return Object.keys(articlesMeta)
    .map((id) => ({ id, ...articlesMeta[id] }))
    .reverse()
}

// 按需加载文章 rawMarkdown（带缓存）
const cache = {}
export async function loadArticle(articleId) {
  const meta = articlesMeta[articleId]
  if (!meta) return null

  if (cache[articleId]) {
    return { id: articleId, meta, content: parseMarkdown(cache[articleId]), rawMarkdown: cache[articleId] }
  }

  try {
    // Vite base 下用相对路径 fetch content/*.md
    const base = import.meta.env.BASE_URL // /FlowerSea-Blog/
    const url = `${base}content/${articleId}.md`
    const response = await fetch(url)
    if (!response.ok) throw new Error(`Failed to load ${articleId}.md`)
    const md = await response.text()
    cache[articleId] = md
    return { id: articleId, meta, content: parseMarkdown(md), rawMarkdown: md }
  } catch (error) {
    console.error('Error loading article:', error)
    return { id: articleId, meta, content: '<p>文章加载失败，请稍后再试。</p>', rawMarkdown: '' }
  }
}

// 从 rawMarkdown 生成目录（h2/h3）
export function generateTOC(mdContent) {
  if (!mdContent) return []
  const lines = mdContent.split('\n')
  const headings = []
  for (const line of lines) {
    const m = line.match(/^(#{2,3})\s+(.+)$/)
    if (m) {
      const level = m[1].length
      const text = m[2].replace(/\*\*(.+?)\*\*/g, '$1').replace(/\*(.+?)\*/g, '$1').trim()
      const id = 'toc-' + text.replace(/[^\w\u4e00-\u9fff]+/g, '-').replace(/^-+|-+$/g, '').toLowerCase()
      headings.push({ level, text, id })
    }
  }
  return headings.length >= 3 ? headings : []
}
