# 音乐文件夹

将你的MP3文件放在这个文件夹里！

## 使用方法

### 1. 添加音乐文件
将MP3文件直接放到 `music` 文件夹中，文件名随意，例如：
```
music/
├── 我的歌1.mp3
├── 晴天.mp3
└── 水星记.mp3
```

### 2. 修改播放列表
打开 `js/player.js` 文件，找到 `playlist` 数组，按以下格式添加：

```javascript
const playlist = [
    {
        title: '晴天',
        artist: '周杰伦',
        file: '../music/晴天.mp3',   // 注意路径！
        cover: 'https://p1.music.126.net/JyvwG1mLtI3-6O1Q6K5x6Q==/109951163665253335.jpg'
    },
    {
        title: '水星记',
        artist: '郭顶',
        file: '../music/水星记.mp3', // 注意路径！
        cover: 'https://p1.music.126.net/7nVVHxlxaX0xD1zTqZ8M2A==/109951163033744672.jpg'
    }
];
```

### 3. 字段说明
- `title` - 歌曲名称
- `artist` - 歌手名称  
- `file` - 音乐文件路径（因为player.js在js文件夹，所以要写 ../music/文件名.mp3）
- `cover` - 封面图片URL（可选，不填则显示默认图标）

### 4. 注意事项
- 支持的格式：MP3、WAV、OGG
- 封面图片可以使用网络URL，也可以放到项目里用相对路径
- 如果不加封面，会显示默认的音乐图标
- 路径一定要写对：`../music/你的歌曲名.mp3`

---

有问题随时修改 js/player.js 即可！
