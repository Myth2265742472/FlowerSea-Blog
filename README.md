# FlowerSea's Blog

> 代码与创造 — 一个基于纯前端技术构建的个人技术博客

## 简介

FlowerSea's Blog 是一个使用 HTML、CSS、JavaScript 构建的单页面应用（SPA）博客。无需后端服务器，所有文章通过 Markdown 格式存储，使用 marked.js 实时解析渲染。

## 特性

- **纯前端 SPA**：无需后端，可直接部署到任何静态托管平台
- **Markdown 文章**：使用 Markdown 格式编写文章，marked.js 实时解析
- **目录自动生成**：文章右侧自动生成可点击跳转的目录导航
- **响应式设计**：适配桌面端和移动端
- **音乐播放器**：内置 Apple Music 风格毛玻璃播放器
- **文章分类与归档**：支持按分类、标签、日期归档浏览
- **搜索功能**：支持文章标题、分类、标签搜索

## 技术栈

- HTML5 + CSS3
- Vanilla JavaScript (ES6+)
- marked.js (Markdown 解析)
- highlight.js (代码高亮)

## 目录结构

```
.
├── index.html          # 主页面
├── css/
│   └── style.css       # 样式文件
├── js/
│   ├── router.js       # SPA 路由
│   ├── markdown-loader.js  # Markdown 加载器
│   ├── player.js       # 音乐播放器
│   └── effects.js      # 视觉效果
├── content/            # Markdown 文章目录
│   ├── article1.md
│   ├── article2.md
│   └── ...
├── music/              # 音乐文件
└── pages/              # 其他页面
```

## 文章列表

| 文章 | 分类 | 日期 |
|------|------|------|
| 深入理解 Typecho 博客系统的设计与实现 | 技术教程 | 2026-03-07 |
| 从零开始搭建个人技术博客的完整指南 | 建站教程 | 2026-03-06 |
| Web 前端开发最佳实践与性能优化 | 前端开发 | 2026-03-05 |
| 使用 Git 进行版本控制的实用技巧 | 版本控制 | 2026-03-04 |
| ESP32 蓝牙低功耗(BLE)通信入门 | 物联网 | 2026-06-07 |
| ESP32 网络遥控车开发指南 | 物联网 | 2026-06-07 |
| ESP32 I2S MEMS 麦克风音频采集实战 | 嵌入式开发 | 2026-06-07 |
| ESP32 Flash 下载工具使用指南 | 开发工具 | 2026-06-07 |
| YD-ESP32-S3 MicroPython 开发入门 | 嵌入式开发 | 2026-06-07 |
| 海光杯比赛 - 机械臂视觉检测系统 | 计算机视觉 | 2026-06-07 |
| 基于51单片机及DS18B20温度传感器的数字温度计设计 | 单片机 | 2026-06-07 |
| ESP32 视觉识别项目开发指南 | 计算机视觉 | 2026-06-07 |
| ESP32 + MPU6050 姿态检测与可视化系统 | 传感器 | 2026-06-11 |
| ESP32 舵机控制全攻略：从基础到高级应用 | 嵌入式开发 | 2026-06-11 |

## 本地运行

由于浏览器的 CORS 安全策略，无法直接通过 `file://` 协议打开。请使用本地服务器：

```bash
# 使用 Python
python -m http.server 8080

# 或使用 Node.js
npx serve .

# 或使用 VS Code Live Server 插件
```

然后访问 `http://localhost:8080`

## 部署

本项目为纯静态网站，可部署到以下平台：

- [GitHub Pages](https://pages.github.com/)
- [Vercel](https://vercel.com/)
- [Netlify](https://www.netlify.com/)
- [InfinityFree](https://www.infinityfree.com/)

## 开源协议

MIT License

## 作者

FlowerSea — [GitHub](https://github.com/Myth2265742472)
