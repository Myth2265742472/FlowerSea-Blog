/**
 * 音乐播放器 - 底部隐藏式毛玻璃风格
 */

(function() {
    // 音乐文件基础URL
    const MUSIC_BASE_URL = 'https://mythlj2.lovestoblog.com/music/';

    // 播放列表
    const playlist = [
        { title: 'Daisy Crown', artist: 'Empty old City,Wuthering Waves', file: 'Empty old City,Wuthering Waves - Daisy Crown.mp3' },
        { title: '该从何处寻你', artist: '巴音孟克 Maj7_Music', file: '巴音孟克 Maj7_Music - 该从何处寻你.mp3' },
        { title: '月半小夜曲', artist: '陈乐基 Rocky Chan', file: '陈乐基 Rocky Chan - 月半小夜曲.mp3' },
        { title: '麦恩莉', artist: '方大同', file: '方大同 - 麦恩莉.mp3' },
        { title: '相机', artist: '加木', file: '加木 - 相机.mp3' },
        { title: '鱼', artist: '加木', file: '加木 - 鱼.mp3' },
        { title: '出现又离开', artist: '梁博', file: '梁博 - 出现又离开 (Live).mp3' },
        { title: '吻得太逼真', artist: '刘大拿,Wiz_H张子豪', file: '刘大拿,Wiz_H张子豪 - 吻得太逼真.mp3' },
        { title: '戒不掉', artist: '欧阳耀莹', file: '欧阳耀莹 - 戒不掉（原声版）.mp3' },
        { title: '爱错', artist: '王力宏', file: '王力宏 - 爱错.mp3' },
        { title: 'Layla蕾拉', artist: '张杰', file: '张杰 - Layla蕾拉.mp3' },
        { title: '酷爱', artist: '张敬轩', file: '张敬轩 - 酷爱.mp3' },
        { title: '雨夜街头', artist: 'Mikey-18,07Kevin,豪一鸽', file: 'Mikey-18,07Kevin,豪一鸽 - 雨夜街头.mp3' }
    ];

    // 状态变量
    let currentIndex = 0;
    let isPlaying = false;
    let audio = null;
    let playerReady = false;
    let hideTimer = null;

    // 初始化播放器
    function init() {
        audio = new Audio();
        audio.crossOrigin = 'anonymous';
        audio.volume = 0.5;
        audio.preload = 'none';

        audio.addEventListener('ended', onEnded);
        audio.addEventListener('error', onError);
        audio.addEventListener('canplay', onCanPlay);
        audio.addEventListener('play', () => {
            isPlaying = true;
            updatePlayButton(true);
            updateCoverAnimation(true);
        });
        audio.addEventListener('pause', () => {
            isPlaying = false;
            updatePlayButton(false);
            updateCoverAnimation(false);
        });
        audio.addEventListener('timeupdate', updateProgress);

        createPlayerUI();
        setupHoverBehavior();
        console.log('音乐播放器已初始化');
    }

    // 创建播放器UI
    function createPlayerUI() {
        const existingPlayer = document.getElementById('music-player');
        if (existingPlayer) existingPlayer.remove();

        const existingStyle = document.getElementById('player-style');
        if (existingStyle) existingStyle.remove();

        const song = playlist[currentIndex];

        const playerHTML = `
            <div id="music-player">
                <div class="player-main">
                    <div class="player-cover">
                        <div class="cover-art">
                            <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="1.2">
                                <circle cx="12" cy="12" r="10"/>
                                <polygon points="10 8 16 12 10 16 10 8"/>
                            </svg>
                        </div>
                        <div class="cover-glow"></div>
                    </div>
                    <div class="player-info">
                        <div class="player-title">${song.title}</div>
                        <div class="player-artist">${song.artist}</div>
                    </div>
                </div>
                <div class="player-progress">
                    <div class="progress-bar">
                        <div class="progress-fill"></div>
                        <div class="progress-handle"></div>
                    </div>
                    <div class="time-display">
                        <span class="current-time">0:00</span>
                        <span class="total-time">0:00</span>
                    </div>
                </div>
                <div class="player-controls">
                    <button class="control-btn btn-prev" onclick="window.musicPlayer.prev()" title="上一首">
                        <svg viewBox="0 0 24 24" fill="currentColor"><path d="M6 6h2v12H6zm3.5 6l8.5 6V6z"/></svg>
                    </button>
                    <button class="control-btn btn-play" onclick="window.musicPlayer.toggle()" title="播放/暂停">
                        <svg class="icon-play" viewBox="0 0 24 24" fill="currentColor"><path d="M8 5v14l11-7z"/></svg>
                        <svg class="icon-pause" viewBox="0 0 24 24" fill="currentColor" style="display:none"><path d="M6 4h4v16H6V4zm8 0h4v16h-4V4z"/></svg>
                    </button>
                    <button class="control-btn btn-next" onclick="window.musicPlayer.next()" title="下一首">
                        <svg viewBox="0 0 24 24" fill="currentColor"><path d="M18 6h2v12h-2zm-9.5 6l-8.5-6v12l8.5-6z"/></svg>
                    </button>
                </div>
                <div class="player-volume">
                    <svg class="volume-icon" viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
                        <polygon points="11 5 6 9 2 9 2 15 6 15 11 19 11 5"/>
                    </svg>
                    <div class="volume-slider">
                        <div class="volume-fill"></div>
                        <input type="range" min="0" max="1" step="0.01" value="0.5" oninput="window.musicPlayer.setVolume(this.value)">
                    </div>
                </div>
            </div>
        `;

        const container = document.createElement('div');
        container.id = 'music-player-container';
        container.innerHTML = playerHTML;

        const style = document.createElement('style');
        style.id = 'player-style';
        style.textContent = `
            #music-player-container {
                position: fixed;
                bottom: 0;
                left: 0;
                right: 0;
                z-index: 9999;
                font-family: -apple-system, BlinkMacSystemFont, 'SF Pro Display', 'Segoe UI', sans-serif;
                transform: translateY(calc(100% - 6px));
                transition: transform 0.4s cubic-bezier(0.16, 1, 0.3, 1);
            }

            #music-player-container.visible {
                transform: translateY(0);
            }

            #music-player-container::before {
                content: '';
                position: absolute;
                top: -20px;
                left: 0;
                right: 0;
                height: 20px;
                z-index: -1;
            }

            #music-player {
                width: 100%;
                max-width: 600px;
                margin: 0 auto;
                background: rgba(28, 28, 30, 0.6);
                backdrop-filter: blur(40px) saturate(180%);
                -webkit-backdrop-filter: blur(40px) saturate(180%);
                border-radius: 20px 20px 0 0;
                border: 1px solid rgba(255, 255, 255, 0.1);
                border-bottom: none;
                padding: 16px 24px 20px;
                box-shadow:
                    0 -10px 40px rgba(0, 0, 0, 0.3),
                    0 0 0 1px rgba(255, 255, 255, 0.05) inset;
                display: flex;
                align-items: center;
                gap: 20px;
            }

            .player-main {
                display: flex;
                align-items: center;
                gap: 12px;
                flex-shrink: 0;
            }

            .player-cover {
                position: relative;
                flex-shrink: 0;
            }

            .cover-art {
                width: 44px;
                height: 44px;
                background: linear-gradient(135deg, #6366f1 0%, #8b5cf6 50%, #06b6d4 100%);
                border-radius: 10px;
                display: flex;
                align-items: center;
                justify-content: center;
                position: relative;
                z-index: 2;
                box-shadow: 0 4px 12px rgba(99, 102, 241, 0.35);
                transition: transform 0.3s cubic-bezier(0.16, 1, 0.3, 1);
            }

            .cover-art svg {
                width: 20px;
                height: 20px;
                color: white;
                opacity: 0.9;
            }

            .cover-art.playing {
                animation: coverPulse 2s ease-in-out infinite;
            }

            @keyframes coverPulse {
                0%, 100% { transform: scale(1); }
                50% { transform: scale(1.03); }
            }

            .cover-glow {
                position: absolute;
                top: 50%;
                left: 50%;
                width: 44px;
                height: 44px;
                background: linear-gradient(135deg, #6366f1, #06b6d4);
                border-radius: 10px;
                transform: translate(-50%, -50%);
                filter: blur(16px);
                opacity: 0.5;
                z-index: 1;
                transition: opacity 0.3s;
            }

            .player-cover:hover .cover-glow {
                opacity: 0.8;
            }

            .player-info {
                width: 120px;
                flex-shrink: 0;
            }

            .player-title {
                color: #ffffff;
                font-size: 14px;
                font-weight: 600;
                letter-spacing: -0.01em;
                white-space: nowrap;
                overflow: hidden;
                text-overflow: ellipsis;
                margin-bottom: 2px;
            }

            .player-artist {
                color: rgba(255, 255, 255, 0.5);
                font-size: 12px;
                font-weight: 400;
                white-space: nowrap;
                overflow: hidden;
                text-overflow: ellipsis;
            }

            .player-progress {
                flex: 1;
                min-width: 0;
            }

            .progress-bar {
                position: relative;
                width: 100%;
                height: 4px;
                background: rgba(255, 255, 255, 0.1);
                border-radius: 2px;
                cursor: pointer;
                overflow: visible;
            }

            .progress-fill {
                position: absolute;
                top: 0;
                left: 0;
                height: 100%;
                background: linear-gradient(90deg, #6366f1, #8b5cf6);
                border-radius: 2px;
                width: 0%;
                transition: width 0.1s linear;
            }

            .progress-handle {
                position: absolute;
                top: 50%;
                left: 0%;
                width: 12px;
                height: 12px;
                background: #ffffff;
                border-radius: 50%;
                transform: translate(-50%, -50%) scale(0);
                box-shadow: 0 2px 8px rgba(0, 0, 0, 0.3);
                transition: transform 0.2s cubic-bezier(0.16, 1, 0.3, 1);
                pointer-events: none;
            }

            .progress-bar:hover .progress-handle {
                transform: translate(-50%, -50%) scale(1);
            }

            .time-display {
                display: flex;
                justify-content: space-between;
                margin-top: 6px;
                font-size: 11px;
                color: rgba(255, 255, 255, 0.35);
                font-variant-numeric: tabular-nums;
            }

            .player-controls {
                display: flex;
                align-items: center;
                gap: 12px;
                flex-shrink: 0;
            }

            .control-btn {
                border: none;
                background: transparent;
                cursor: pointer;
                display: flex;
                align-items: center;
                justify-content: center;
                transition: all 0.2s cubic-bezier(0.16, 1, 0.3, 1);
                padding: 0;
                color: rgba(255, 255, 255, 0.7);
            }

            .control-btn:hover {
                color: #ffffff;
                transform: scale(1.1);
            }

            .control-btn:active {
                transform: scale(0.95);
            }

            .control-btn svg {
                width: 20px;
                height: 20px;
            }

            .btn-play {
                width: 36px;
                height: 36px;
                background: linear-gradient(135deg, #6366f1, #8b5cf6);
                border-radius: 50%;
                color: #ffffff;
                box-shadow: 0 4px 12px rgba(99, 102, 241, 0.4);
            }

            .btn-play:hover {
                transform: scale(1.08);
                box-shadow: 0 6px 20px rgba(99, 102, 241, 0.5);
            }

            .btn-play svg {
                width: 14px;
                height: 14px;
                margin-left: 1px;
            }

            .player-volume {
                display: flex;
                align-items: center;
                gap: 8px;
                flex-shrink: 0;
                width: 100px;
            }

            .volume-icon {
                width: 16px;
                height: 16px;
                color: rgba(255, 255, 255, 0.4);
                flex-shrink: 0;
            }

            .volume-slider {
                position: relative;
                flex: 1;
                height: 4px;
                background: rgba(255, 255, 255, 0.1);
                border-radius: 2px;
            }

            .volume-fill {
                position: absolute;
                top: 0;
                left: 0;
                height: 100%;
                width: 50%;
                background: rgba(255, 255, 255, 0.4);
                border-radius: 2px;
                pointer-events: none;
            }

            .volume-slider input[type="range"] {
                position: absolute;
                top: -6px;
                left: 0;
                width: 100%;
                height: 16px;
                -webkit-appearance: none;
                appearance: none;
                background: transparent;
                cursor: pointer;
                margin: 0;
            }

            .volume-slider input[type="range"]::-webkit-slider-thumb {
                -webkit-appearance: none;
                width: 12px;
                height: 12px;
                background: #ffffff;
                border-radius: 50%;
                box-shadow: 0 2px 6px rgba(0, 0, 0, 0.3);
                cursor: pointer;
                transition: transform 0.2s;
            }

            .volume-slider input[type="range"]::-webkit-slider-thumb:hover {
                transform: scale(1.2);
            }

            .volume-slider input[type="range"]::-moz-range-thumb {
                width: 12px;
                height: 12px;
                background: #ffffff;
                border-radius: 50%;
                border: none;
                box-shadow: 0 2px 6px rgba(0, 0, 0, 0.3);
                cursor: pointer;
            }

            /* 移动端适配 */
            @media (max-width: 640px) {
                #music-player {
                    padding: 12px 16px 16px;
                    gap: 12px;
                }
                .player-info {
                    width: 80px;
                }
                .player-volume {
                    width: 80px;
                }
            }

            @media (max-width: 480px) {
                .player-progress {
                    display: none;
                }
                .player-volume {
                    display: none;
                }
            }
        `;

        document.head.appendChild(style);
        document.body.appendChild(container);

        updatePlayButton(false);
        updateVolumeUI(0.5);
    }

    // 设置悬停行为
    function setupHoverBehavior() {
        const container = document.getElementById('music-player-container');
        if (!container) return;

        // 鼠标移入显示
        container.addEventListener('mouseenter', () => {
            clearTimeout(hideTimer);
            container.classList.add('visible');
        });

        // 鼠标移出延迟隐藏
        container.addEventListener('mouseleave', () => {
            hideTimer = setTimeout(() => {
                container.classList.remove('visible');
            }, 800);
        });

        // 页面底部悬停检测（20px触发区域）
        let triggerZone = document.createElement('div');
        triggerZone.style.cssText = `
            position: fixed;
            bottom: 0;
            left: 0;
            right: 0;
            height: 20px;
            z-index: 9998;
        `;
        document.body.appendChild(triggerZone);

        triggerZone.addEventListener('mouseenter', () => {
            clearTimeout(hideTimer);
            container.classList.add('visible');
        });
    }

    // 播放指定歌曲
    function playSong(index) {
        if (index < 0) index = playlist.length - 1;
        if (index >= playlist.length) index = 0;

        currentIndex = index;
        const song = playlist[currentIndex];
        const src = MUSIC_BASE_URL + song.file;

        const titleEl = document.querySelector('.player-title');
        const artistEl = document.querySelector('.player-artist');
        if (titleEl) titleEl.textContent = song.title;
        if (artistEl) artistEl.textContent = song.artist;

        resetProgress();

        audio.src = src;
        audio.load();
        audio.play().then(() => {
            isPlaying = true;
            updatePlayButton(true);
            updateCoverAnimation(true);
        }).catch(err => {
            console.error('播放失败:', err);
        });
    }

    // 切换播放/暂停
    function toggle() {
        if (!audio.src) {
            playSong(currentIndex);
            return;
        }

        if (isPlaying) {
            audio.pause();
        } else {
            audio.play().catch(err => {
                console.error('播放失败:', err);
                playSong(currentIndex);
            });
        }
    }

    // 播放下一首
    function next() {
        playSong(currentIndex + 1);
    }

    // 播放上一首
    function prev() {
        playSong(currentIndex - 1);
    }

    // 设置音量
    function setVolume(value) {
        audio.volume = value;
        updateVolumeUI(value);
    }

    // 更新音量UI
    function updateVolumeUI(value) {
        const fill = document.querySelector('.volume-fill');
        if (fill) {
            fill.style.width = (value * 100) + '%';
        }
    }

    // 更新进度条
    function updateProgress() {
        if (!audio.duration) return;

        const percent = (audio.currentTime / audio.duration) * 100;
        const fill = document.querySelector('.progress-fill');
        const handle = document.querySelector('.progress-handle');
        const currentTimeEl = document.querySelector('.current-time');
        const totalTimeEl = document.querySelector('.total-time');

        if (fill) fill.style.width = percent + '%';
        if (handle) handle.style.left = percent + '%';
        if (currentTimeEl) currentTimeEl.textContent = formatTime(audio.currentTime);
        if (totalTimeEl) totalTimeEl.textContent = formatTime(audio.duration);
    }

    // 重置进度
    function resetProgress() {
        const fill = document.querySelector('.progress-fill');
        const handle = document.querySelector('.progress-handle');
        const currentTimeEl = document.querySelector('.current-time');
        const totalTimeEl = document.querySelector('.total-time');

        if (fill) fill.style.width = '0%';
        if (handle) handle.style.left = '0%';
        if (currentTimeEl) currentTimeEl.textContent = '0:00';
        if (totalTimeEl) totalTimeEl.textContent = '0:00';
    }

    // 格式化时间
    function formatTime(seconds) {
        if (isNaN(seconds)) return '0:00';
        const mins = Math.floor(seconds / 60);
        const secs = Math.floor(seconds % 60);
        return mins + ':' + (secs < 10 ? '0' : '') + secs;
    }

    // 更新播放按钮
    function updatePlayButton(playing) {
        const iconPlay = document.querySelector('.icon-play');
        const iconPause = document.querySelector('.icon-pause');
        if (iconPlay && iconPause) {
            iconPlay.style.display = playing ? 'none' : 'block';
            iconPause.style.display = playing ? 'block' : 'none';
        }
    }

    // 更新封面动画
    function updateCoverAnimation(playing) {
        const cover = document.querySelector('.cover-art');
        if (cover) {
            cover.classList.toggle('playing', playing);
        }
    }

    // 播放结束
    function onEnded() {
        next();
    }

    // 加载完成
    function onCanPlay() {
        playerReady = true;
        const totalTimeEl = document.querySelector('.total-time');
        if (totalTimeEl) totalTimeEl.textContent = formatTime(audio.duration);
    }

    // 错误处理
    function onError(e) {
        console.error('音频错误:', e);
        console.error('错误URL:', audio.src);
        isPlaying = false;
        updatePlayButton(false);
        updateCoverAnimation(false);
    }

    // 暴露全局方法
    window.musicPlayer = {
        toggle: toggle,
        next: next,
        prev: prev,
        setVolume: setVolume,
        play: function(index) {
            if (index !== undefined) {
                playSong(index);
            } else {
                toggle();
            }
        }
    };

    // 页面加载完成后初始化
    if (document.readyState === 'loading') {
        document.addEventListener('DOMContentLoaded', init);
    } else {
        init();
    }
})();
