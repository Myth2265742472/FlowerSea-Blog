/**
 * SPA Router — FlowerSea Blog (Dark Theme Edition)
 */

// Page title map
const pageTitleMap = {
    home: "FlowerSea's Blog — 代码与创造",
    post: '文章详情 — FlowerSea',
    article: '文章详情 — FlowerSea',
    about: '关于 — FlowerSea',
    archive: '归档 — FlowerSea'
};

let currentPage = 'home';
let currentArticleId = null;

// Get article index for prev/next nav
function getArticleIds() {
    return Object.keys(window.markdownLoader ? window.markdownLoader.getArticleList() : []).reverse();
}

// Generate Home Page (used for Home, Category, Archive, and Search results)
function getHomeTemplate(pageNum = 1, filterType = null, filterValue = null) {
    let allArticles = (window.markdownLoader ? window.markdownLoader.getArticleList() : []).slice().reverse();
    
    // Apply filters
    let pageTitle = "Blog Posts";
    let sectionTag = "// 最近更新";
    
    if (filterType === 'category' && filterValue) {
        allArticles = allArticles.filter(a => a.category === filterValue);
        pageTitle = `分类: ${filterValue}`;
        sectionTag = "// 分类筛选";
    } else if (filterType === 'archive' && filterValue) {
        allArticles = allArticles.filter(a => {
            let m = a.date.match(/(\d+)年(\d+)月/);
            if (!m) m = a.date.match(/(\d{4})-(\d{2})/);
            
            if (m) {
                const monthStr = parseInt(m[2], 10).toString();
                const k = m[1] + '年' + monthStr + '月';
                return k === filterValue;
            }
            return false;
        });
        pageTitle = `归档: ${filterValue}`;
        sectionTag = "// 归档筛选";
    } else if (filterType === 'search' && filterValue) {
        const q = filterValue.toLowerCase();
        allArticles = allArticles.filter(a => 
            a.title.toLowerCase().includes(q) || 
            a.category.toLowerCase().includes(q) ||
            (a.tags && a.tags.some(t => t.toLowerCase().includes(q)))
        );
        pageTitle = `搜索: "${filterValue}"`;
        sectionTag = "// 搜索结果";
    }

    // Pagination logic
    const pageSize = 5;
    const totalPages = Math.ceil(allArticles.length / pageSize) || 1;
    const currentPage = Math.max(1, Math.min(pageNum, totalPages));
    const articles = allArticles.slice((currentPage - 1) * pageSize, currentPage * pageSize);

    const cards = articles.map(a => {
        const summary = a.summary || '';
        return `
        <article class="post-card" data-tilt>
            <div class="post-card-glow"></div>
            <div class="post-card-inner">
                <div class="post-category"><span class="category-dot"></span><a href="#" onclick="navigateTo('home', null, 1, false, 'category', '${a.category}');return false;" style="color:inherit;text-decoration:none;">${a.category}</a></div>
                <h2><a href="#" onclick="navigateTo('article','${a.id}');return false;">${a.title}</a></h2>
                <div class="article-meta-new">
                    <span>FlowerSea</span>
                    <span>${a.date}</span>
                </div>
                <div class="article-excerpt-wrapper">
                    <p class="article-excerpt-toggle" onclick="toggleExcerpt('${a.id}')">
                        <span class="excerpt-text">点击阅读全文</span>
                        <svg class="excerpt-arrow" width="14" height="14" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M5 12h14M12 5l7 7-7 7"/></svg>
                    </p>
                    <div class="article-excerpt-content" id="excerpt-${a.id}">
                        <p>${summary}</p>
                    </div>
                </div>
                <div class="post-footer-new">
                    <div class="post-meta"><span class="post-date">${a.date}</span></div>
                    <a href="#" onclick="navigateTo('article','${a.id}');return false;" class="post-link-new magnetic">
                        Read
                        <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M5 12h14M12 5l7 7-7 7"/></svg>
                    </a>
                </div>
            </div>
        </article>
    `}).join('');

    // Pagination Controls
    let paginationHTML = '<div class="pagination">';
    if (currentPage > 1) {
        paginationHTML += `<a href="#" onclick="navigateTo('home', null, ${currentPage - 1}, true, '${filterType || ''}', '${filterValue || ''}');return false;" class="page-btn">← 上一页</a>`;
    }
    paginationHTML += `<span class="page-info">第 ${currentPage} / ${totalPages} 页</span>`;
    if (currentPage < totalPages) {
        paginationHTML += `<a href="#" onclick="navigateTo('home', null, ${currentPage + 1}, true, '${filterType || ''}', '${filterValue || ''}');return false;" class="page-btn">下一页 →</a>`;
    }
    paginationHTML += '</div>';

    return `
        <div id="posts-list">
            <div class="section-header reveal">
                <span class="section-tag">${sectionTag}</span>
                <h2 class="section-title">${pageTitle} <span style="font-size: 0.5em; color: var(--text-muted); font-weight: normal; margin-left: 10px;">(共 ${allArticles.length} 篇)</span></h2>
            </div>
            ${filterType ? `<div style="margin-bottom: 20px;"><a href="#" onclick="navigateTo('home');return false;" class="page-btn" style="display:inline-block; font-size: 13px;">清除筛选 ✕</a></div>` : ''}
            <div class="posts-grid">
                ${cards || '<div class="loading">暂无相关文章</div>'}
            </div>
            ${totalPages > 1 ? paginationHTML : ''}
        </div>
    `;
}

// Generate Archive Page
function getArchiveTemplate() {
    const articles = (window.markdownLoader ? window.markdownLoader.getArticleList() : []).slice().reverse();
    
    const items = articles.map(a => `
        <div class="archive-item">
            <a href="#" onclick="navigateTo('article','${a.id}');return false;">${a.title}</a>
            <span class="date">${a.date}</span>
        </div>
    `).join('');

    return `
        <div class="archive-page">
            <div class="section-header reveal">
                <span class="section-tag">// 文章归档</span>
                <h2 class="section-title">Archive</h2>
            </div>
            ${items || '<div class="loading">加载中</div>'}
        </div>
    `;
}

// Generate About Page
function getAboutTemplate() {
    return `
        <div class="about-page">
            <div class="about-hero-card reveal">
                <div class="about-avatar-circle"><span>👨‍💻</span></div>
                <div class="about-intro-text">
                    <h3>FlowerSea</h3>
                    <div class="role">Full-Stack Engineer · Open Source Lover</div>
                    <p>你好，欢迎来到我的个人博客！我是一名热爱技术的开发者，喜欢探索新技术，也喜欢将所学知识分享给他人。这个博客是我记录学习心得、分享技术经验的地方。</p>
                    <p>我相信代码可以改变世界，也相信分享能让知识更有价值。</p>
                    <div class="social-links">
                        <a href="https://github.com/Myth2265742472" target="_blank" class="magnetic">🐙 GitHub</a>
                        <a href="https://juejin.cn/user/960332145889899" target="_blank" class="magnetic">📝 掘金</a>
                        <a href="mailto:2265742472@qq.com" class="magnetic">📧 Email</a>
                    </div>
                </div>
            </div>

            <div class="about-section-card reveal">
                <div class="about-section-title"><div class="icon-box icon-blue">✦</div>博客内容</div>
                <ul>
                    <li><span style="color:var(--accent)">▸</span> Web 前端开发技巧与最佳实践</li>
                    <li><span style="color:var(--accent)">▸</span> 后端技术学习笔记</li>
                    <li><span style="color:var(--accent)">▸</span> 开源项目的使用与二次开发</li>
                    <li><span style="color:var(--accent)">▸</span> 开发工具和效率提升方法</li>
                    <li><span style="color:var(--accent)">▸</span> 技术书籍和教程的读后感</li>
                    <li><span style="color:var(--accent)">▸</span> 生活随笔与思考</li>
                </ul>
            </div>

            <div class="about-section-card reveal">
                <div class="about-section-title"><div class="icon-box icon-cyan">⚡</div>技术栈</div>
                <ul>
                    <li><span style="color:var(--accent-2)">▸</span> <strong>前端</strong> — HTML5, CSS3, JS, Vue.js, React</li>
                    <li><span style="color:var(--accent-2)">▸</span> <strong>后端</strong> — PHP, Node.js, Python</li>
                    <li><span style="color:var(--accent-2)">▸</span> <strong>数据库</strong> — MySQL, PostgreSQL, MongoDB, Redis</li>
                    <li><span style="color:var(--accent-2)">▸</span> <strong>DevOps</strong> — Docker, Nginx, CI/CD, Linux</li>
                    <li><span style="color:var(--accent-2)">▸</span> <strong>工具</strong> — Git, VS Code, Figma</li>
                    <li><span style="color:var(--accent-2)">▸</span> <strong>其他</strong> — 持续学习中...</li>
                </ul>
            </div>

            <div class="contact-cta reveal">
                <h3>一起聊聊？</h3>
                <p>无论是技术问题、项目合作还是随便聊聊，都欢迎联系我</p>
                <a href="mailto:2265742472@qq.com" class="email-box magnetic">2265742472@qq.com</a>
                <div>
                    <a href="mailto:2265742472@qq.com" class="btn btn-primary magnetic">
                        <span>发送邮件</span>
                        <svg width="18" height="18" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M5 12h14M12 5l7 7-7 7"/></svg>
                    </a>
                </div>
            </div>
        </div>
    `;
}

// Generate Article Detail Page
function getArticleHTML(article) {
    if (!article) return '<div class="loading">文章加载失败</div>';

    const ids = getArticleIds();
    const idx = ids.indexOf(article.id);
    const prevId = idx > 0 ? ids[idx - 1] : null;
    const nextId = idx < ids.length - 1 ? ids[idx + 1] : null;
    const prevMeta = prevId && window.markdownLoader ? window.markdownLoader.getArticleList().find(a => a.id === prevId) : null;
    const nextMeta = nextId && window.markdownLoader ? window.markdownLoader.getArticleList().find(a => a.id === nextId) : null;

    // Generate table of contents from raw markdown content
    const toc = generateTOC(article.rawMarkdown || article.content);

    return `
        <div class="article-detail">
            <div class="article-header">
                <a href="#" onclick="navigateTo('home');return false;" style="display:inline-flex;align-items:center;gap:8px;color:var(--text-muted);font-size:13px;font-family:var(--font-mono);margin-bottom:20px;">
                    <svg width="16" height="16" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2"><path d="M19 12H5M12 19l-7-7 7-7"/></svg>
                    返回首页
                </a>
                <h1 class="article-title">${article.meta.title}</h1>
                <div class="article-info">
                    <span>FlowerSea</span>
                    <span class="dot"></span>
                    <span>${article.meta.date}</span>
                    <span class="dot"></span>
                    <span>${article.meta.category}</span>
                    <span class="dot"></span>
                    <span>💬 ${article.meta.comments}</span>
                </div>
            </div>
            <div class="article-content-wrapper">
                <div class="article-body prose">${article.content}</div>
                ${toc ? `<aside class="article-toc" id="article-toc">${toc}</aside>` : ''}
            </div>
            <div class="article-tags">
                <span class="tag-label">#</span>
                ${article.meta.tags.map(t => `<a href="#" class="magnetic">${t}</a>`).join('')}
            </div>
            <div class="article-nav">
                ${prevMeta ? `<div class="article-nav-item"><div class="nav-label">← 上一篇</div><a href="#" onclick="navigateTo('article','${prevId}');return false;">${prevMeta.title}</a></div>` : '<div></div>'}
                ${nextMeta ? `<div class="article-nav-item" style="text-align:right;"><div class="nav-label">下一篇 →</div><a href="#" onclick="navigateTo('article','${nextId}');return false;">${nextMeta.title}</a></div>` : '<div></div>'}
            </div>
        </div>
    `;
}

// Navigate
async function navigateTo(pageName, articleId = null, pageNum = 1, isPagination = false, filterType = null, filterValue = null) {
    const contentArea = document.getElementById('content-area');
    const hero = document.getElementById('hero');
    if (!contentArea) return;

    // Fade out
    contentArea.style.opacity = '0';
    contentArea.style.transform = 'translateY(10px)';

    setTimeout(async () => {
        // Show/hide hero
        if (hero) hero.classList.toggle('hidden', pageName !== 'home');

        // Show/hide sidebar (hide on article pages for full-width reading)
        const sidebar = document.getElementById('sidebar');
        if (sidebar) {
            if (pageName === 'article') {
                sidebar.style.display = 'none';
                contentArea.parentElement.style.gridTemplateColumns = '1fr';
            } else {
                sidebar.style.display = '';
                contentArea.parentElement.style.gridTemplateColumns = '';
            }
        }

        if (pageName === 'article' && articleId) {
            currentArticleId = articleId;
            const article = await window.markdownLoader.loadArticle(articleId);
            contentArea.innerHTML = getArticleHTML(article);
            document.title = `${article ? article.meta.title : '文章'} — FlowerSea`;
            
            // For articles, scroll to the top of the article content, not the very top of the page
            const articleEl = document.querySelector('.article-detail');
            if (articleEl) {
                const y = articleEl.getBoundingClientRect().top + window.scrollY - 80;
                window.scrollTo({ top: y, behavior: 'smooth' });
            } else {
                window.scrollTo(0, 0);
            }
            
            // Initialize TOC after article content is rendered
            setTimeout(initArticleTOC, 100);
        } else if (pageName === 'home') {
            contentArea.innerHTML = getHomeTemplate(pageNum, filterType, filterValue);
            document.title = pageTitleMap.home;
        } else if (pageName === 'archive') {
            contentArea.innerHTML = getArchiveTemplate();
            document.title = pageTitleMap.archive;
        } else if (pageName === 'about') {
            contentArea.innerHTML = getAboutTemplate();
            document.title = pageTitleMap.about;
        } else if (pageName === 'post') {
            currentArticleId = 'article1';
            const article = await window.markdownLoader.loadArticle('article1');
            contentArea.innerHTML = getArticleHTML(article);
            document.title = pageTitleMap.post;
        } else {
            contentArea.innerHTML = getHomeTemplate(pageNum, filterType, filterValue);
            document.title = pageTitleMap.home;
        }

        // Fade in
        contentArea.style.opacity = '1';
        contentArea.style.transform = 'translateY(0)';

        // Update nav
        document.querySelectorAll('.nav-link').forEach(l => { l.classList.remove('active'); if (l.dataset.page === pageName) l.classList.add('active'); });

        // Scroll logic is now handled in individual page branches to avoid overriding
        if (pageName !== 'article') {
            if (isPagination || filterType) {
                // When paginating OR when applying a filter (search/category/archive),
                // scroll to the top of the posts list (main content area).
                const postsList = document.getElementById('posts-list');
                const mainContent = document.getElementById('content-area');
                const targetEl = postsList || mainContent;
                
                if (targetEl) {
                    // Scroll to the top of the target element, accounting for the fixed navbar (approx 80px)
                    const y = targetEl.getBoundingClientRect().top + window.scrollY - 80;
                    window.scrollTo({ top: y, behavior: 'smooth' });
                } else {
                    window.scrollTo(0, 0);
                }
            } else if (pageName === 'home' && !filterType) {
                 // For pure home navigation, leave it at the top to see the hero, or let user scroll
                 window.scrollTo(0, 0);
            } else {
                 window.scrollTo(0, 0);
            }
        }

        currentPage = pageName;

        // Update URL
        let url = '?page=' + pageName;
        if (articleId) url += '&id=' + articleId;
        if (pageName === 'home' && pageNum > 1) url += '&p=' + pageNum;
        if (filterType && filterValue) {
            url += `&filter=${filterType}&val=${encodeURIComponent(filterValue)}`;
        }
        history.pushState({ page: pageName, articleId, pageNum, filterType, filterValue }, '', url);
    }, 250);
}

// Generate Table of Contents from raw markdown content
function generateTOC(mdContent) {
    if (!mdContent) return '';
    const lines = mdContent.split('\n');
    const headings = [];
    
    for (const line of lines) {
        // Match ## and ### headings (h2 and h3)
        const match = line.match(/^(#{2,3})\s+(.+)$/);
        if (match) {
            const level = match[1].length;
            const text = match[2].replace(/\*\*(.+?)\*\*/g, '$1').replace(/\*(.+?)\*/g, '$1').trim();
            // Create a slug for the heading
            const id = 'toc-' + text.replace(/[^\w\u4e00-\u9fff]+/g, '-').replace(/^-+|-+$/g, '').toLowerCase();
            headings.push({ level, text, id });
        }
    }
    
    if (headings.length < 3) return ''; // Only show TOC if 3+ headings
    
    let html = '<div class="toc-title">目录</div><ul class="toc-list">';
    for (const h of headings) {
        const indent = h.level === 3 ? ' toc-h3' : '';
        html += `<li class="toc-item${indent}"><a href="#${h.id}" onclick="event.preventDefault();scrollToHeading('${h.id}')">${h.text}</a></li>`;
    }
    html += '</ul>';
    return html;
}

// Scroll to heading and update TOC active state
function scrollToHeading(id) {
    const el = document.getElementById(id);
    if (!el) return;
    const y = el.getBoundingClientRect().top + window.scrollY - 80;
    window.scrollTo({ top: y, behavior: 'smooth' });
}

// Initialize TOC: add IDs to headings and setup scroll tracking
function initArticleTOC() {
    const prose = document.querySelector('.article-body.prose');
    if (!prose) return;
    
    // Add IDs to all h2 and h3 elements
    const headings = prose.querySelectorAll('h2, h3');
    headings.forEach(h => {
        const text = h.textContent.trim();
        const id = 'toc-' + text.replace(/[^\w\u4e00-\u9fff]+/g, '-').replace(/^-+|-+$/g, '').toLowerCase();
        h.id = id;
    });
    
    // Setup scroll spy for TOC
    const tocLinks = document.querySelectorAll('.toc-item a');
    if (tocLinks.length === 0) return;
    
    function updateActiveTOC() {
        const scrollY = window.scrollY;
        let activeId = '';
        
        headings.forEach(h => {
            if (h.getBoundingClientRect().top + scrollY - 100 <= scrollY) {
                activeId = h.id;
            }
        });
        
        tocLinks.forEach(link => {
            const href = link.getAttribute('href');
            if (href === '#' + activeId) {
                link.classList.add('toc-active');
            } else {
                link.classList.remove('toc-active');
            }
        });
    }
    
    window.addEventListener('scroll', updateActiveTOC, { passive: true });
    updateActiveTOC();
}

// Toggle article excerpt expand/collapse
function toggleExcerpt(articleId) {
    const content = document.getElementById('excerpt-' + articleId);
    const toggle = content.previousElementSibling;
    const arrow = toggle.querySelector('.excerpt-arrow');
    const text = toggle.querySelector('.excerpt-text');
    
    if (content.classList.contains('expanded')) {
        content.classList.remove('expanded');
        toggle.classList.remove('active');
        text.textContent = '点击阅读全文';
    } else {
        // Close other expanded excerpts
        document.querySelectorAll('.article-excerpt-content.expanded').forEach(el => {
            el.classList.remove('expanded');
            const otherToggle = el.previousElementSibling;
            if (otherToggle) {
                otherToggle.classList.remove('active');
                const otherText = otherToggle.querySelector('.excerpt-text');
                if (otherText) otherText.textContent = '点击阅读全文';
            }
        });
        
        content.classList.add('expanded');
        toggle.classList.add('active');
        text.textContent = '收起摘要';
    }
}

// Search
function handleSearch() {
    const q = document.getElementById('search-input').value.trim();
    if (!q) return;
    
    // Reset search input visually
    document.getElementById('search-input').value = '';
    
    // Navigate to home page with search filter
    navigateTo('home', null, 1, false, 'search', q);
}

// Update sidebar data
function updateSidebar() {
    const articles = window.markdownLoader ? window.markdownLoader.getArticleList() : [];
    if (!articles.length) return;

    // Latest Articles
    const latestEl = document.getElementById('latest-articles');
    if (latestEl) {
        // Reverse to get the latest first, then slice top 5
        const latestArticles = articles.slice().reverse().slice(0, 5);
        latestEl.innerHTML = latestArticles.map(a => `<li><a href="#" onclick="navigateTo('article', '${a.id}'); return false;">${a.title}</a></li>`).join('');
    }

    // Categories
    const cats = {};
    articles.forEach(a => cats[a.category] = (cats[a.category] || 0) + 1);
    const catEl = document.getElementById('category-list');
    if (catEl) catEl.innerHTML = Object.entries(cats).map(([c, n]) => `<li><a href="#" onclick="navigateTo('home', null, 1, false, 'category', '${c}'); return false;">${c} (${n})</a></li>`).join('');

    // Archive
    const arcs = {};
    articles.forEach(a => { 
        // 支持两种日期格式: "2026年3月7日" 和 "2026-03-07"
        let m = a.date.match(/(\d+)年(\d+)月/); 
        if (!m) {
            m = a.date.match(/(\d{4})-(\d{2})/);
        }
        
        if (m) { 
            // 统一转换成 "2026年3月" 格式进行统计和展示
            const monthStr = parseInt(m[2], 10).toString(); // 去掉前导0
            const k = m[1] + '年' + monthStr + '月'; 
            arcs[k] = (arcs[k] || 0) + 1; 
        } 
    });
    const arcEl = document.getElementById('archive-list');
    if (arcEl) {
        const sorted = Object.entries(arcs).sort((a, b) => new Date(b[0].replace(/年/, '-').replace(/月/, '-01')) - new Date(a[0].replace(/年/, '-').replace(/月/, '-01')));
        arcEl.innerHTML = sorted.map(([m, n]) => `<li><a href="#" onclick="navigateTo('home', null, 1, false, 'archive', '${m}'); return false;">${m} (${n})</a></li>`).join('');
    }
}

// Init
async function initRouter() {
    // Transition style
    const s = document.createElement('style');
    s.textContent = `#content-area { transition: opacity 0.3s ease, transform 0.3s ease; }`;
    document.head.appendChild(s);

    // Update Hero Stats dynamically
    const updateHeroStats = () => {
        // Calculate days running from the earliest article date
        const articles = window.markdownLoader ? window.markdownLoader.getArticleList() : [];
        const totalArticles = articles.length;
        
        // Parse all article dates and find the earliest one
        const parsedDates = articles.map(a => {
            let m = a.date.match(/(\d+)年(\d+)月(\d+)日/);
            if (m) return new Date(parseInt(m[1]), parseInt(m[2]) - 1, parseInt(m[3]));
            m = a.date.match(/(\d{4})-(\d{2})-(\d{2})/);
            if (m) return new Date(parseInt(m[1]), parseInt(m[2]) - 1, parseInt(m[3]));
            return null;
        }).filter(d => d !== null);
        
        const launchDate = parsedDates.length > 0 ? new Date(Math.min(...parsedDates)) : new Date();
        const daysRunning = Math.floor((new Date() - launchDate) / (1000 * 60 * 60 * 24));
        
        // Update DOM elements before counter animation starts
        const statEls = document.querySelectorAll('.stat-number');
        if (statEls.length >= 3) {
            statEls[0].dataset.target = totalArticles; // 篇文章
            statEls[1].dataset.target = daysRunning;   // 天运行
            
            // Try to fetch public repos as "commits/projects" or fallback to a calculated number
            fetch('https://api.github.com/users/Myth2265742472')
                .then(res => res.json())
                .then(data => {
                    if (data && data.public_repos !== undefined) {
                        statEls[2].dataset.target = data.public_repos; // Use public repos
                        const labelEl = statEls[2].nextElementSibling;
                        if (labelEl) labelEl.textContent = '个开源项目';
                    } else {
                        statEls[2].dataset.target = totalArticles * 12; // fallback mock
                    }
                })
                .catch(() => {
                    statEls[2].dataset.target = totalArticles * 12; // fallback mock
                });
        }
    };
    updateHeroStats();

    // Check URL params
    const params = new URLSearchParams(window.location.search);
    const page = params.get('page');
    const id = params.get('id');
    const p = parseInt(params.get('p')) || 1;
    const filter = params.get('filter');
    const val = params.get('val');

    if (page === 'article' && id) await navigateTo('article', id);
    else if (page === 'post') await navigateTo('post');
    else if (page === 'about') await navigateTo('about');
    else if (page === 'archive') await navigateTo('archive');
    else await navigateTo('home', null, p, false, filter, val);

    updateSidebar();

    window.addEventListener('popstate', e => {
        if (e.state && e.state.page) navigateTo(e.state.page, e.state.articleId, e.state.pageNum || 1, false, e.state.filterType, e.state.filterValue);
    });
}

if (document.readyState === 'loading') document.addEventListener('DOMContentLoaded', initRouter);
else initRouter();
