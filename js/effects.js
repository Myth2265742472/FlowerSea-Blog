/**
 * FlowerSea's Blog — Visual Effects
 * Particles, cursor, typed, counters, scroll reveal, reading progress
 */

document.addEventListener('DOMContentLoaded', () => {
    const prefersReducedMotion = window.matchMedia('(prefers-reduced-motion: reduce)').matches;
    const isCoarsePointer = window.matchMedia('(pointer: coarse)').matches;

    // ====== Particles 优化版 (性能优化) ======
    const canvas = document.getElementById('particles');
    if (canvas && !prefersReducedMotion) {
        const ctx = canvas.getContext('2d');
        let particles = [];
        let mouse = { x: -1000, y: -1000 };
        const COUNT = Math.min(window.innerWidth < 768 || isCoarsePointer ? 28 : 64, 100); // 移动端减少粒子数量
        const CONNECT = 140;
        let animationId = null;
        let isTabActive = true;

        function resize() { 
            canvas.width = window.innerWidth; 
            canvas.height = window.innerHeight; 
            // 调整粒子数量基于屏幕大小
            if (window.innerWidth < 768 || isCoarsePointer) {
                particles = particles.slice(0, Math.min(particles.length, 28));
            }
        }
        resize();
        window.addEventListener('resize', resize);

        class Particle {
            constructor() {
                this.x = Math.random() * canvas.width;
                this.y = Math.random() * canvas.height;
                this.vx = (Math.random() - 0.5) * 0.4;
                this.vy = (Math.random() - 0.5) * 0.4;
                this.r = Math.random() * 1.5 + 0.5; // 减小粒子大小
                this.a = Math.random() * 0.4 + 0.1; // 降低透明度
            }
            update() {
                this.x += this.vx;
                this.y += this.vy;
                if (this.x < 0 || this.x > canvas.width) this.vx *= -1;
                if (this.y < 0 || this.y > canvas.height) this.vy *= -1;
                const dx = this.x - mouse.x;
                const dy = this.y - mouse.y;
                const d = Math.sqrt(dx * dx + dy * dy);
                if (d < 120) { 
                    const f = (120 - d) / 120 * 0.015; 
                    this.vx += dx * f; 
                    this.vy += dy * f; 
                }
                this.vx *= 0.99; 
                this.vy *= 0.99;
            }
            draw() { 
                ctx.beginPath(); 
                ctx.arc(this.x, this.y, this.r, 0, Math.PI * 2); 
                ctx.fillStyle = `rgba(99,102,241,${this.a})`; 
                ctx.fill(); 
            }
        }

        // 批量创建粒子
        for (let i = 0; i < COUNT; i++) {
            particles.push(new Particle());
        }

        function animate() {
            if (!isTabActive) {
                animationId = requestAnimationFrame(animate);
                return;
            }
            
            ctx.clearRect(0, 0, canvas.width, canvas.height);
            
            // 优化：使用for循环代替forEach
            for (let i = 0; i < particles.length; i++) {
                particles[i].update();
                particles[i].draw();
            }
            
            // 优化：减少连接线计算
            for (let i = 0; i < particles.length; i++) {
                for (let j = i + 1; j < particles.length; j++) {
                    const dx = particles[i].x - particles[j].x;
                    const dy = particles[i].y - particles[j].y;
                    const dSquared = dx * dx + dy * dy;
                    
                    if (dSquared < CONNECT * CONNECT) {
                        const d = Math.sqrt(dSquared);
                        ctx.beginPath();
                        ctx.moveTo(particles[i].x, particles[i].y);
                        ctx.lineTo(particles[j].x, particles[j].y);
                        ctx.strokeStyle = `rgba(99,102,241,${(1 - d / CONNECT) * 0.08})`; // 降低连接线透明度
                        ctx.lineWidth = 0.3;
                        ctx.stroke();
                    }
                }
            }
            
            animationId = requestAnimationFrame(animate);
        }

        function startAnimation() {
            if (!animationId) {
                animate();
            }
        }

        function stopAnimation() {
            if (animationId) {
                cancelAnimationFrame(animationId);
                animationId = null;
            }
        }

        // 启动动画
        startAnimation();

        // 页面可见性变化时优化性能
        document.addEventListener('visibilitychange', () => {
            isTabActive = !document.hidden;
            if (isTabActive) {
                startAnimation();
            } else {
                stopAnimation();
            }
        });

        // 鼠标交互
        document.addEventListener('mousemove', e => { 
            mouse.x = e.clientX; 
            mouse.y = e.clientY; 
        });
        document.addEventListener('mouseleave', () => { 
            mouse.x = -1000; 
            mouse.y = -1000; 
        });

        // 优化：处理窗口卸载
        window.addEventListener('beforeunload', stopAnimation);
    }

    // ====== Cursor 优化版 ======
    const dot = document.getElementById('cursor-dot');
    const ring = document.getElementById('cursor-ring');
    if (dot && ring && !prefersReducedMotion) {
        let mx = 0, my = 0, rx = 0, ry = 0;
        let animationId = null;
        let isMobile = isCoarsePointer || /Android|webOS|iPhone|iPad|iPod|BlackBerry|IEMobile|Opera Mini/i.test(navigator.userAgent);
        
        // 移动端禁用光标效果
        if (isMobile) {
            dot.style.display = 'none';
            ring.style.display = 'none';
        } else {
            // 优化鼠标移动事件
            const handleMouseMove = (e) => { 
                mx = e.clientX; 
                my = e.clientY; 
                dot.style.left = mx + 'px'; 
                dot.style.top = my + 'px'; 
            };
            
            // 使用requestAnimationFrame优化动画
            function animateCursor() {
                rx += (mx - rx) * 0.15;
                ry += (my - ry) * 0.15;
                ring.style.left = rx + 'px';
                ring.style.top = ry + 'px';
                animationId = requestAnimationFrame(animateCursor);
            }
            
            // 节流的事件监听器
            let mouseMoveTimeout = null;
            document.addEventListener('mousemove', (e) => {
                if (!mouseMoveTimeout) {
                    handleMouseMove(e);
                    mouseMoveTimeout = setTimeout(() => {
                        mouseMoveTimeout = null;
                    }, 16); // ~60fps
                }
            });
            
            // 启动光标动画
            animateCursor();
            
            // 优化悬停效果
            const hoverElementsSelector = 'a, button, .magnetic, .post-card, .widget, .article-nav-item, .archive-item';
            let hoverElements = [];
            
            function updateHoverElements() {
                hoverElements = Array.from(document.querySelectorAll(hoverElementsSelector));
            }
            
            function handleCursorEnter() { 
                dot.classList.add('hover'); 
                ring.classList.add('hover'); 
            }
            
            function handleCursorLeave() { 
                dot.classList.remove('hover'); 
                ring.classList.remove('hover'); 
            }
            
            function attachHoverListeners() {
                hoverElements.forEach(el => {
                    el.addEventListener('mouseenter', handleCursorEnter);
                    el.addEventListener('mouseleave', handleCursorLeave);
                });
            }
            
            function detachHoverListeners() {
                hoverElements.forEach(el => {
                    el.removeEventListener('mouseenter', handleCursorEnter);
                    el.removeEventListener('mouseleave', handleCursorLeave);
                });
            }
            
            // 初始设置
            updateHoverElements();
            attachHoverListeners();
            
            // SPA导航后重新绑定
            const origNav = window.navigateTo;
            if (origNav) {
                const origFn = origNav;
                window.navigateTo = async function(...args) {
                    await origFn.apply(this, args);
                    setTimeout(() => {
                        detachHoverListeners();
                        updateHoverElements();
                        attachHoverListeners();
                    }, 300);
                };
            }
            
            // 窗口卸载时清理
            window.addEventListener('beforeunload', () => {
                if (animationId) {
                    cancelAnimationFrame(animationId);
                }
                detachHoverListeners();
            });
        }
    }

    // ====== Navbar scroll 优化版 ======
    const navbar = document.getElementById('navbar');
    if (navbar) {
        let scrollTimeout = null;
        let lastScrollY = window.scrollY;
        
        function handleScroll() {
            const currentScrollY = window.scrollY;
            
            // 添加滚动方向检测
            if (currentScrollY > lastScrollY && currentScrollY > 100) {
                // 向下滚动超过100px
                navbar.classList.add('scrolled', 'scrolling-down');
                navbar.classList.remove('scrolling-up');
            } else if (currentScrollY < lastScrollY) {
                // 向上滚动
                navbar.classList.add('scrolled', 'scrolling-up');
                navbar.classList.remove('scrolling-down');
            }
            
            // 基础滚动状态
            navbar.classList.toggle('scrolled', currentScrollY > 50);
            
            lastScrollY = currentScrollY;
            
            // 清除之前的超时
            if (scrollTimeout) {
                clearTimeout(scrollTimeout);
            }
            
            // 滚动停止后移除滚动方向类
            scrollTimeout = setTimeout(() => {
                navbar.classList.remove('scrolling-down', 'scrolling-up');
            }, 150);
        }
        
        // 使用节流的滚动事件
        let ticking = false;
        window.addEventListener('scroll', () => {
            if (!ticking) {
                requestAnimationFrame(() => {
                    handleScroll();
                    ticking = false;
                });
                ticking = true;
            }
        });
        
        // 初始检查
        handleScroll();
    }

    // ====== Mobile menu 优化版 ======
    const menuToggle = document.getElementById('menu-toggle');
    const mainNav = document.getElementById('main-nav');
    if (menuToggle && mainNav) {
        let isMenuOpen = false;
        
        function toggleMenu() {
            isMenuOpen = !isMenuOpen;
            mainNav.classList.toggle('open');
            menuToggle.setAttribute('aria-expanded', isMenuOpen);
            
            // 添加/移除滚动锁定
            if (isMenuOpen) {
                document.body.style.overflow = 'hidden';
                menuToggle.classList.add('active');
            } else {
                document.body.style.overflow = '';
                menuToggle.classList.remove('active');
            }
        }
        
        function closeMenu() {
            if (isMenuOpen) {
                isMenuOpen = false;
                mainNav.classList.remove('open');
                menuToggle.setAttribute('aria-expanded', false);
                document.body.style.overflow = '';
                menuToggle.classList.remove('active');
            }
        }
        
        // 点击菜单按钮
        menuToggle.addEventListener('click', (e) => {
            e.stopPropagation();
            toggleMenu();
        });
        
        // 点击菜单链接关闭菜单
        mainNav.querySelectorAll('a').forEach(a => {
            a.addEventListener('click', (e) => {
                e.stopPropagation();
                closeMenu();
            });
        });
        
        // 点击外部区域关闭菜单
        document.addEventListener('click', (e) => {
            if (isMenuOpen && !mainNav.contains(e.target) && e.target !== menuToggle) {
                closeMenu();
            }
        });
        
        // ESC键关闭菜单
        document.addEventListener('keydown', (e) => {
            if (isMenuOpen && e.key === 'Escape') {
                closeMenu();
            }
        });
        
        // 窗口大小变化时关闭菜单
        window.addEventListener('resize', () => {
            if (window.innerWidth > 768) {
                closeMenu();
            }
        });
    }

    // ====== Typed Effect 优化版 ======
    const typedEl = document.getElementById('typed-text');
    if (typedEl && prefersReducedMotion) {
        typedEl.textContent = '记录思考，分享创造';
    } else if (typedEl) {
        const phrases = ['全栈工程师 / 开源爱好者', '用代码构建数字世界', '探索技术的无限可能', '记录思考，分享创造'];
        let pi = 0, ci = 0, del = false, wait = 0;
        let animationId = null;
        let isTabActive = true;
        
        function typeEffect() {
            if (!isTabActive) {
                animationId = requestAnimationFrame(typeEffect);
                return;
            }
            
            if (wait > 0) { 
                wait--; 
                animationId = requestAnimationFrame(typeEffect); 
                return; 
            }
            
            const cur = phrases[pi];
            if (!del) { 
                typedEl.textContent = cur.slice(0, ++ci); 
                if (ci === cur.length) { 
                    del = true; 
                    wait = 100; // 完整显示后的等待时间
                } 
            } else { 
                typedEl.textContent = cur.slice(0, --ci); 
                if (ci === 0) { 
                    del = false; 
                    pi = (pi + 1) % phrases.length;
                    wait = 50; // 切换到下一句前的等待时间
                } 
            }
            
            // 根据是否在删除调整速度
            const speed = del ? 25 : 50;
            setTimeout(() => {
                animationId = requestAnimationFrame(typeEffect);
            }, speed);
        }
        
        // 页面可见性控制
        document.addEventListener('visibilitychange', () => {
            isTabActive = !document.hidden;
            if (isTabActive && !animationId) {
                animationId = requestAnimationFrame(typeEffect);
            }
        });
        
        // 延迟启动，让页面先加载
        setTimeout(() => {
            animationId = requestAnimationFrame(typeEffect);
        }, 1200);
        
        // 清理函数
        window.addEventListener('beforeunload', () => {
            if (animationId) {
                cancelAnimationFrame(animationId);
            }
        });
    }

    // ====== Counter 优化版 ======
    const counters = document.querySelectorAll('.stat-number');
    if (counters.length) {
        let observer = null;
        
        // 使用IntersectionObserver的懒加载
        if ('IntersectionObserver' in window) {
            observer = new IntersectionObserver((entries) => {
                entries.forEach(entry => {
                    if (entry.isIntersecting) {
                        const el = entry.target;
                        const target = parseInt(el.dataset.target);
                        if (!el._animated) {
                            animateCounter(el, target);
                            el._animated = true;
                            observer.unobserve(el);
                        }
                    }
                });
            }, { 
                threshold: 0.3,
                rootMargin: '50px' // 提前50px开始观察
            });
            
            counters.forEach(counter => {
                observer.observe(counter);
            });
        } else {
            // 回退方案：直接动画
            counters.forEach(counter => {
                const target = parseInt(counter.dataset.target);
                animateCounter(counter, target);
            });
        }
        
        function animateCounter(element, target) {
            let current = 0;
            const increment = target / 60; // 60帧完成动画
            const duration = 1500; // 1.5秒完成
            const stepTime = duration / 60;
            
            function updateCounter() {
                // Get the latest target in case it was updated dynamically
                const realTarget = parseInt(element.dataset.target) || 0;
                
                current += Math.max(realTarget / 60, 1);
                if (current >= realTarget) {
                    current = realTarget;
                    element.textContent = formatNumber(realTarget);
                    return;
                }
                
                element.textContent = formatNumber(Math.floor(current));
                setTimeout(updateCounter, stepTime);
            }
            
            // 使用requestAnimationFrame启动
            requestAnimationFrame(() => {
                updateCounter();
            });
        }
        
        function formatNumber(num) {
            if (num >= 1000) {
                return Math.floor(num / 1000) + 'k+';
            }
            return num.toString();
        }
        
        // 清理观察器
        window.addEventListener('beforeunload', () => {
            if (observer) {
                observer.disconnect();
            }
        });
    }

    // ====== Reveal 优化版 ======
    let revealObserver = null;
    let revealElements = [];
    
    function initReveal() {
        // 清理旧的观察器
        if (revealObserver) {
            revealObserver.disconnect();
        }
        
        // 获取新的元素
        revealElements = Array.from(document.querySelectorAll('.reveal:not(.visible)'));
        
        if (revealElements.length === 0) return;
        
        // 创建新的观察器
        if ('IntersectionObserver' in window) {
            revealObserver = new IntersectionObserver((entries) => {
                entries.forEach((entry, index) => {
                    if (entry.isIntersecting) {
                        // 使用requestAnimationFrame优化性能
                        requestAnimationFrame(() => {
                            // 添加延迟创建交错动画效果
                            setTimeout(() => {
                                entry.target.classList.add('visible');
                            }, index * 50); // 交错延迟
                        });
                        
                        revealObserver.unobserve(entry.target);
                    }
                });
            }, { 
                threshold: 0.1,
                rootMargin: '50px' // 提前50px触发
            });
            
            // 观察所有元素
            revealElements.forEach(el => {
                revealObserver.observe(el);
            });
        } else {
            // 回退方案：直接显示所有元素
            revealElements.forEach((el, index) => {
                setTimeout(() => {
                    el.classList.add('visible');
                }, index * 100);
            });
        }
    }
    
    // 初始化和优化
    setTimeout(initReveal, 100); // 延迟初始化，避免阻塞主线程
    
    // 滚动时重新检查可见元素
    let scrollTimer = null;
    window.addEventListener('scroll', () => {
        if (scrollTimer) {
            clearTimeout(scrollTimer);
        }
        scrollTimer = setTimeout(initReveal, 150);
    });
    
    // 窗口大小变化时重新检查
    window.addEventListener('resize', () => {
        setTimeout(initReveal, 100);
    });
    
    // SPA导航后重新初始化
    const origNav2 = window.navigateTo;
    if (origNav2) {
        const orig = origNav2;
        window.navigateTo = async function(...args) {
            await orig.apply(this, args);
            setTimeout(initReveal, 100);
        };
    }
    
    // 清理观察器
    window.addEventListener('beforeunload', () => {
        if (revealObserver) {
            revealObserver.disconnect();
        }
    });

    // ====== Reading Progress ===
    window.addEventListener('scroll', () => {
        const h = document.documentElement;
        const pct = (h.scrollTop / (h.scrollHeight - h.clientHeight)) * 100;
        const bar = document.getElementById('reading-progress');
        if (bar) bar.style.width = Math.min(pct, 100) + '%';
    });

    // ====== 3D Card Tilt Effect ======
    function init3DCards() {
        if (prefersReducedMotion || isCoarsePointer) return;
        const cardSelectors = [
            { selector: '.post-card', gradientClass: 'post-card-mouse-gradient', tilt3d: true },
            { selector: '.widget', gradientClass: 'widget-mouse-gradient', tilt3d: false },
            { selector: '.about-hero-card', gradientClass: 'about-hero-gradient', tilt3d: false },
            { selector: '.about-section-card', gradientClass: 'about-section-gradient', tilt3d: false },
            { selector: '.archive-item', gradientClass: 'archive-item-gradient', tilt3d: false }
        ];
        
        cardSelectors.forEach(({ selector, gradientClass, tilt3d }) => {
            const cards = document.querySelectorAll(selector);
            
            cards.forEach(card => {
                let isHovering = false;
                let targetRotateX = 0;
                let targetRotateY = 0;
                
                let gradientEl = card.querySelector('.' + gradientClass);
                if (!gradientEl) {
                    gradientEl = document.createElement('div');
                    gradientEl.className = gradientClass;
                    card.appendChild(gradientEl);
                }
                
                const handleMouseMove = (e) => {
                    if (!isHovering) return;
                    
                    const rect = card.getBoundingClientRect();
                    const centerX = rect.left + rect.width / 2;
                    const centerY = rect.top + rect.height / 2;
                    
                    const mouseX = e.clientX - centerX;
                    const mouseY = e.clientY - centerY;
                    
                    if (tilt3d) {
                        const rotateX = (mouseY / (rect.height / 2)) * -8;
                        const rotateY = (mouseX / (rect.width / 2)) * 8;
                        targetRotateX = rotateX;
                        targetRotateY = rotateY;
                        card.style.setProperty('--rotateX', targetRotateX + 'deg');
                        card.style.setProperty('--rotateY', targetRotateY + 'deg');
                    }
                    
                    const mouseXPercent = ((e.clientX - rect.left) / rect.width * 100).toFixed(2);
                    const mouseYPercent = ((e.clientY - rect.top) / rect.height * 100).toFixed(2);
                    gradientEl.style.setProperty('--mouse-x', mouseXPercent + '%');
                    gradientEl.style.setProperty('--mouse-y', mouseYPercent + '%');
                };
                
                const handleMouseEnter = () => {
                    isHovering = true;
                    if (tilt3d) {
                        card.classList.add('tilt-3d');
                    }
                    document.addEventListener('mousemove', handleMouseMove);
                };
                
                const handleMouseLeave = () => {
                    isHovering = false;
                    if (tilt3d) {
                        card.classList.remove('tilt-3d');
                        card.style.setProperty('--rotateX', '0deg');
                        card.style.setProperty('--rotateY', '0deg');
                    }
                    document.removeEventListener('mousemove', handleMouseMove);
                };
                
                card.addEventListener('mouseenter', handleMouseEnter);
                card.addEventListener('mouseleave', handleMouseLeave);
                
                card._cleanup = () => {
                    card.removeEventListener('mouseenter', handleMouseEnter);
                    card.removeEventListener('mouseleave', handleMouseLeave);
                    document.removeEventListener('mousemove', handleMouseMove);
                };
            });
        });
    }
    
    setTimeout(init3DCards, 300);
    
    const origNav3DCards = window.navigateTo;
    if (origNav3DCards) {
        window.navigateTo = async function(...args) {
            await origNav3DCards.apply(this, args);
            setTimeout(init3DCards, 500);
        };
    }

});
