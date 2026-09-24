<script setup>
import { onMounted, onUnmounted, ref, computed } from 'vue'
import { playlist } from '../data/blog.js'

const audio = ref(null)
const currentIndex = ref(0)
const isPlaying = ref(false)
const currentTime = ref(0)
const duration = ref(0)
const volume = ref(0.7)
const showVolume = ref(false)

const current = computed(() => playlist[currentIndex.value] || {})
const progress = computed(() => duration.value ? (currentTime.value / duration.value) * 100 : 0)
const fmt = (s) => {
  if (!s || isNaN(s)) return '0:00'
  const m = Math.floor(s / 60), sec = Math.floor(s % 60)
  return `${m}:${sec < 10 ? '0' + sec : sec}`
}

const src = computed(() => {
  const base = import.meta.env.BASE_URL
  return `${base}music/${current.value.file}`
})

const play = () => {
  if (!audio.value) return
  audio.value.play().then(() => (isPlaying.value = true)).catch(() => {})
}
const toggle = () => { isPlaying.value ? audio.value.pause() : play() }
const next = () => { currentIndex.value = (currentIndex.value + 1) % playlist.length; autoNext() }
const prev = () => { currentIndex.value = (currentIndex.value - 1 + playlist.length) % playlist.length; autoNext() }
const autoNext = () => { isPlaying.value = true; setTimeout(() => play(), 100) }

const onTime = () => { currentTime.value = audio.value.currentTime }
const onMeta = () => { duration.value = audio.value.duration || 0 }
const onEnd = () => next()
const seek = (e) => {
  const rect = e.currentTarget.getBoundingClientRect()
  const pct = (e.clientX - rect.left) / rect.width
  if (audio.value && duration.value) audio.value.currentTime = pct * duration.value
}
const setVolume = (e) => {
  const rect = e.currentTarget.getBoundingClientRect()
  const pct = (e.clientX - rect.left) / rect.width
  volume.value = Math.max(0, Math.min(1, pct))
  if (audio.value) audio.value.volume = volume.value
}

onMounted(() => {
  if (audio.value) audio.value.volume = volume.value
})
onUnmounted(() => { audio.value?.pause() })
</script>

<template>
  <div id="music-player-container" class="visible">
    <div id="music-player">
      <audio ref="audio" :src="src" @timeupdate="onTime" @loadedmetadata="onMeta" @ended="onEnd"></audio>

      <div class="player-main">
        <div class="cover-art" :class="{ playing: isPlaying }">
          <div class="cover-glow"></div>
          <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
            <circle cx="12" cy="12" r="3" />
            <path d="M9 18V5l12-2v13" />
          </svg>
        </div>
        <div class="player-info">
          <div class="player-title">{{ current.title }}</div>
          <div class="player-artist">{{ current.artist }}</div>
        </div>
      </div>

      <div class="player-progress" @click="seek">
        <div class="progress-bar"><div class="progress-fill" :style="{ width: progress + '%' }"></div></div>
        <div class="time-display"><span>{{ fmt(currentTime) }}</span><span>{{ fmt(duration) }}</span></div>
      </div>

      <div class="player-controls">
        <button class="control-btn" @click="prev" aria-label="上一首">
          <svg viewBox="0 0 24 24" fill="currentColor"><path d="M6 6h2v12H6zm3.5 6l8.5 6V6z" /></svg>
        </button>
        <button class="control-btn btn-play" @click="toggle" :aria-label="isPlaying ? '暂停' : '播放'">
          <svg v-if="!isPlaying" viewBox="0 0 24 24" fill="currentColor"><path d="M8 5v14l11-7z" /></svg>
          <svg v-else viewBox="0 0 24 24" fill="currentColor"><path d="M6 5h4v14H6zM14 5h4v14h-4z" /></svg>
        </button>
        <button class="control-btn" @click="next" aria-label="下一首">
          <svg viewBox="0 0 24 24" fill="currentColor"><path d="M6 18l8.5-6L6 6v12zM16 6v12h2V6z" /></svg>
        </button>
      </div>

      <div class="player-volume" @click="setVolume" @mouseenter="showVolume = true" @mouseleave="showVolume = false">
        <svg viewBox="0 0 24 24" fill="none" stroke="currentColor" stroke-width="2">
          <path d="M11 5L6 9H2v6h4l5 4V5z" />
          <path v-if="volume > 0.3" d="M19.07 4.93a10 10 0 010 14.14M15.54 8.46a5 5 0 010 7.07" />
        </svg>
      </div>
    </div>
  </div>
</template>

<style scoped>
#music-player-container { position: fixed; bottom: 24px; right: 24px; z-index: 9996; max-width: 380px; transform: translateY(0); transition: transform 0.4s cubic-bezier(0.16,1,0.3,1); }
#music-player-container:not(.visible) { transform: translateY(120%); }
#music-player { display: flex; align-items: center; gap: 16px; padding: 12px 16px; background: rgba(18,18,26,0.92); backdrop-filter: blur(24px) saturate(180%); -webkit-backdrop-filter: blur(24px) saturate(180%); border: 1px solid rgba(255,255,255,0.06); border-radius: 100px; box-shadow: 0 10px 40px rgba(0,0,0,0.4); color: #e4e4e7; font-family: 'JetBrains Mono', monospace; }
.player-main { display: flex; align-items: center; gap: 12px; }
.cover-art { position: relative; width: 44px; height: 44px; border-radius: 50%; background: linear-gradient(135deg, #6366f1, #06b6d4); display: flex; align-items: center; justify-content: center; flex-shrink: 0; color: white; }
.cover-art.playing { animation: spin 8s linear infinite; }
.cover-glow { position: absolute; inset: -4px; border-radius: 50%; background: radial-gradient(circle, rgba(99,102,241,0.4), transparent 70%); opacity: 0; transition: opacity 0.4s; }
.cover-art.playing .cover-glow { opacity: 1; }
.player-info { min-width: 120px; }
.player-title { font-size: 13px; font-weight: 700; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; max-width: 160px; }
.player-artist { font-size: 11px; opacity: 0.6; white-space: nowrap; overflow: hidden; text-overflow: ellipsis; max-width: 160px; }
.player-progress { flex: 1; min-width: 120px; cursor: pointer; }
.progress-bar { height: 3px; background: rgba(255,255,255,0.1); border-radius: 2px; overflow: hidden; }
.progress-fill { height: 100%; background: linear-gradient(90deg, #6366f1, #06b6d4); transition: width 0.1s linear; }
.time-display { display: flex; justify-content: space-between; font-size: 10px; opacity: 0.6; margin-top: 4px; }
.player-controls { display: flex; align-items: center; gap: 4px; }
.control-btn { background: none; border: none; color: inherit; cursor: pointer; padding: 6px; display: flex; align-items: center; justify-content: center; border-radius: 50%; transition: all 0.25s; }
.control-btn svg { width: 18px; height: 18px; }
.control-btn:hover { color: #818cf8; background: rgba(255,255,255,0.05); }
.btn-play { background: #6366f1; color: white; width: 32px; height: 32px; }
.btn-play:hover { background: #818cf8; color: white; }
.btn-play svg { width: 14px; height: 14px; }
.player-volume { padding: 8px; cursor: pointer; opacity: 0.6; }
.player-volume svg { width: 18px; height: 18px; }
.player-volume:hover { opacity: 1; }
@keyframes spin { to { transform: rotate(360deg); } }

@media (max-width: 768px) {
  #music-player-container { right: 12px; left: 12px; bottom: 12px; max-width: none; }
}
</style>
