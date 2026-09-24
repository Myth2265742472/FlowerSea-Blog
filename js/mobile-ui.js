/**
 * FlowerSea's Blog — Mobile UI Controller
 *
 * 检测移动端设备访问，启用独立的移动端界面（body.mobile-mode）：
 *   - 顶部仅保留 logo（导航下沉为底部 Tab 栏）
 *   - 紧凑 Hero / 全宽卡片 / 单列布局
 *   - 音乐播放器变为常驻迷你条
 *   - 关闭桌面端光标特效与 3D 倾斜
 *
 * 桌面端访问时本脚本直接 return，不添加 mobile-mode，桌面 UI 完全不变。
 * 样式见 css/mobile.css。
 */
(function () {
    'use strict';

    // ====== 移动端检测：UA 关键字 或 (窄屏 + 触摸设备) ======
    function isMobileDevice() {
        const ua = navigator.userAgent || '';
        const uaMobile = /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(ua);
        const narrowTouch = window.matchMedia('(max-width: 768px)').matches &&
            (navigator.maxTouchPoints > 0 || 'ontouchstart' in window);
        return uaMobile || narrowTouch;
    }

    // 非移动端：什么都不做，桌面 UI 保持原样
    if (!isMobileDevice()) return;

    // 立即打上 mobile-mode 标记（脚本位于 body 末尾，body 已存在）
    document.documentElement.classList.add('mobile-mode');
    if (document.body) {
        document.body.classList.add('mobile-mode');
    } else {
        document.addEventListener('DOMContentLoaded', () => document.body.classList.add('mobile-mode'));
    }

    // ====== 底部 Tab 栏定义 ======
    // stroke 图标（与桌面端导航项一致）
    var TABS = [
        {
            id: 'home', label: '首页', external: false,
            icon: '<path d="M3 12l9-9 9 9M5 10v10h4v-6h6v6h4V10"/>'
        },
        {
            id: 'archive', label: '归档', external: false,
            icon: '<path d="M4 6h16M4 12h16M4 18h10"/>'
        },
        {
            id: 'about', label: '关于', external: false,
            icon: '<circle cx="12" cy="8" r="4"/><path d="M4 20c0-4 4-6 8-6s8 2 8 6"/>'
        },
        {
            id: 'github', label: 'GitHub', external: true,
            icon: '<path d="M9 19c-5 1.5-5-2.5-7-3m14 6v-3.9a3.4 3.4 0 0 0-.9-2.6c3-.3 6.2-1.5 6.2-6.7a5.2 5.2 0 0 0-1.4-3.6 4.8 4.8 0 0 0-.1-3.6s-1.1-.3-3.6 1.4a12.3 12.3 0 0 0-6.4 0C6.8 1.1 5.7 1.4 5.7 1.4a4.8 4.8 0 0 0-.1 3.6 5.2 5.2 0 0 0-1.4 3.6c0 5.2 3.2 6.4 6.2 6.7a3.4 3.4 0 0 0-.9 2.6V23"/>'
        }
    ];

    function createTabBar() {
        if (document.querySelector('.mobile-tabbar')) return;

        var bar = document.createElement('nav');
        bar.className = 'mobile-tabbar';
        bar.setAttribute('aria-label', '主导航');

        bar.innerHTML = TABS.map(function (t) {
            var attr = t.external ? 'data-external="1"' : 'data-page="' + t.id + '"';
            return '<button class="tab" ' + attr + ' aria-label="' + t.label + '" type="button">' +
                '<svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2" ' +
                'stroke-linecap="round" stroke-linejoin="round">' + t.icon + '</svg>' +
                '<span>' + t.label + '</span>' +
                '</button>';
        }).join('');

        document.body.appendChild(bar);

        // 绑定点击
        bar.querySelectorAll('.tab').forEach(function (btn) {
            btn.addEventListener('click', function () {
                if (btn.dataset.external) {
                    window.open('https://github.com/Myth2265742472', '_blank', 'noopener');
                    return;
                }
                var page = btn.dataset.page;
                if (page && typeof window.navigateTo === 'function') {
                    window.navigateTo(page);
                }
            });
        });

        syncActive('home');
    }

    // 同步 Tab 激活态（文章页归到首页上下文）
    function syncActive(page) {
        var p = (page === 'article' || page === 'post') ? 'home' : (page || 'home');
        document.querySelectorAll('.mobile-tabbar .tab').forEach(function (btn) {
            btn.classList.toggle('active', btn.dataset.page === p);
        });
    }

    // 包装 navigateTo：导航完成后同步 Tab 激活态
    function wrapNavigate() {
        var orig = window.navigateTo;
        if (!orig || orig.__mobileWrapped) return;
        var wrapped = async function () {
            var page = arguments[0];
            try {
                await orig.apply(this, arguments);
            } finally {
                syncActive(page);
            }
        };
        wrapped.__mobileWrapped = true;
        window.navigateTo = wrapped;
    }

    function init() {
        createTabBar();
        wrapNavigate();
        // 浏览器前进/后退时根据 URL 同步
        window.addEventListener('popstate', function () {
            var params = new URLSearchParams(window.location.search);
            syncActive(params.get('page') || 'home');
        });
    }

    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
