/**
 * FlowerSea's Blog - 自动化文章生成工具
 * 
 * 使用方法：
 * 1. 将项目文件夹放入 "要上传到网页的文章" 目录
 * 2. 运行此脚本：node blog-auto-tool.js
 * 3. 按提示操作
 * 4. 脚本会自动生成文章并更新配置
 */

const fs = require('fs');
const path = require('path');

// ==================== 配置区域 ====================

const CONFIG = {
    // 项目根目录
    projectRoot: 'C:\\Users\\lj226\\Desktop\\网页测试\\博客测试\\归\\1',
    
    // 文章存放目录
    contentDir: 'content',
    
    // 项目资料目录
    projectsDir: '要上传到网页的文章',
    
    // 配置文件路径
    loaderFile: 'js\\markdown-loader.js',
    
    // 作者信息
    author: 'FlowerSea',
    
    // 默认分类映射
    categoryMap: {
        'esp32': '嵌入式开发',
        'esp32-s3': '嵌入式开发',
        'arduino': '嵌入式开发',
        '51单片机': '单片机',
        'stc89c52': '单片机',
        '树莓派': '物联网',
        'raspberry': '物联网',
        'python': '软件开发',
        'flask': '软件开发',
        'opencv': '计算机视觉',
        '视觉': '计算机视觉',
        '摄像头': '计算机视觉',
        '机械臂': '机器人',
        '舵机': '机器人',
        'mpu6050': '传感器',
        '温度传感器': '传感器',
        'ds18b20': '传感器',
        '麦克风': '音频',
        'i2s': '音频',
        '蓝牙': '物联网',
        'ble': '物联网',
        'wifi': '物联网',
        '遥控车': '物联网',
        'flash': '开发工具',
        '固件': '开发工具',
        'micropython': '嵌入式开发',
        'potplayer': '软件工具',
        'bilibili': '软件工具'
    }
};

// ==================== 工具函数 ====================

/**
 * 扫描项目文件夹
 * @param {string} dirPath - 项目目录路径
 * @returns {object} - 项目信息
 */
function scanProject(dirPath) {
    const projectName = path.basename(dirPath);
    const files = [];
    const codeFiles = [];
    const docFiles = [];
    
    // 递归扫描文件
    function scanDir(currentPath, relativePath = '') {
        const items = fs.readdirSync(currentPath);
        
        for (const item of items) {
            const fullPath = path.join(currentPath, item);
            const relPath = path.join(relativePath, item);
            const stat = fs.statSync(fullPath);
            
            if (stat.isDirectory()) {
                // 跳过特定目录
                if (['node_modules', '.git', 'build', '__pycache__'].includes(item)) {
                    continue;
                }
                scanDir(fullPath, relPath);
            } else {
                files.push(relPath);
                
                // 分类文件
                const ext = path.extname(item).toLowerCase();
                if (['.ino', '.c', '.cpp', '.h', '.py', '.js'].includes(ext)) {
                    codeFiles.push(relPath);
                } else if (['.md', '.txt', '.doc', '.docx'].includes(ext)) {
                    docFiles.push(relPath);
                }
            }
        }
    }
    
    scanDir(dirPath);
    
    return {
        name: projectName,
        path: dirPath,
        files: files,
        codeFiles: codeFiles,
        docFiles: docFiles,
        fileCount: files.length
    };
}

/**
 * 读取代码文件内容
 * @param {string} filePath - 文件路径
 * @returns {string} - 文件内容
 */
function readCodeFile(filePath) {
    try {
        const content = fs.readFileSync(filePath, 'utf-8');
        // 限制长度，避免过大
        return content.length > 5000 ? content.substring(0, 5000) + '\n// ... (代码过长，已截断)' : content;
    } catch (err) {
        return '// 无法读取文件';
    }
}

/**
 * 读取文档文件
 * @param {string} filePath - 文件路径
 * @returns {string} - 文档内容
 */
function readDocFile(filePath) {
    try {
        return fs.readFileSync(filePath, 'utf-8');
    } catch (err) {
        return '';
    }
}

/**
 * 智能识别项目类型
 * @param {object} projectInfo - 项目信息
 * @returns {string} - 项目类型
 */
function detectProjectType(projectInfo) {
    const name = projectInfo.name.toLowerCase();
    const files = projectInfo.files.join(' ').toLowerCase();
    
    // 根据项目名称和文件类型判断
    if (name.includes('esp32') || files.includes('.ino')) {
        if (name.includes('s3') || files.includes('s3')) return 'ESP32-S3';
        return 'ESP32';
    }
    if (name.includes('51') || name.includes('stc89') || files.includes('.c')) {
        return '51单片机';
    }
    if (name.includes('树莓派') || name.includes('raspberry') || name.includes('pi')) {
        return '树莓派';
    }
    if (name.includes('视觉') || name.includes('opencv') || name.includes('camera')) {
        return '视觉识别';
    }
    if (name.includes('机械臂') || name.includes('舵机') || name.includes('servo')) {
        return '机器人';
    }
    if (name.includes('蓝牙') || name.includes('ble')) {
        return '蓝牙通信';
    }
    if (name.includes('wifi') || name.includes('网络')) {
        return '网络通信';
    }
    if (name.includes('温度') || name.includes('ds18b20')) {
        return '温度传感器';
    }
    if (name.includes('麦克风') || name.includes('i2s') || name.includes('audio')) {
        return '音频处理';
    }
    if (name.includes('flash') || name.includes('下载')) {
        return '固件工具';
    }
    if (name.includes('potplayer') || name.includes('bilibili')) {
        return '媒体工具';
    }
    
    return '嵌入式开发';
}

/**
 * 智能生成分类
 * @param {string} projectType - 项目类型
 * @param {string} projectName - 项目名称
 * @returns {string} - 文章分类
 */
function generateCategory(projectType, projectName) {
    const name = projectName.toLowerCase();
    
    // 直接匹配
    for (const [keyword, category] of Object.entries(CONFIG.categoryMap)) {
        if (name.includes(keyword) || projectType.toLowerCase().includes(keyword)) {
            return category;
        }
    }
    
    // 默认分类
    return '技术教程';
}

/**
 * 生成文章标题
 * @param {string} projectName - 项目名称
 * @param {string} projectType - 项目类型
 * @returns {string} - 文章标题
 */
function generateTitle(projectName, projectType) {
    // 清理项目名称
    let title = projectName
        .replace(/[-_]/g, ' ')
        .replace(/\([^)]*\)/g, '')
        .trim();
    
    // 如果标题太短，添加类型说明
    if (title.length < 10) {
        title = `${projectType} - ${title}`;
    }
    
    // 如果标题不包含项目类型，添加前缀
    if (!title.toLowerCase().includes(projectType.toLowerCase())) {
        title = `${projectType} ${title}`;
    }
    
    return title;
}

/**
 * 生成文章摘要
 * @param {object} projectInfo - 项目信息
 * @param {string} projectType - 项目类型
 * @returns {string} - 文章摘要
 */
function generateSummary(projectInfo, projectType) {
    const docContent = projectInfo.docFiles.length > 0 
        ? readDocFile(path.join(projectInfo.path, projectInfo.docFiles[0]))
        : '';
    
    // 从文档中提取前100字作为摘要
    if (docContent.length > 50) {
        return docContent.substring(0, 100).replace(/\n/g, ' ') + '...';
    }
    
    // 生成默认摘要
    return `本项目是一个${projectType}项目，包含${projectInfo.fileCount}个文件。通过详细的代码和说明，帮助开发者快速理解和使用。`;
}

/**
 * 生成标签
 * @param {string} projectName - 项目名称
 * @param {string} projectType - 项目类型
 * @returns {array} - 标签数组
 */
function generateTags(projectName, projectType) {
    const tags = [projectType];
    const name = projectName.toLowerCase();
    
    // 提取关键词作为标签
    const keywords = [
        'ESP32', 'Arduino', 'Python', 'Flask', 'OpenCV',
        '蓝牙', 'WiFi', '传感器', '舵机', '摄像头',
        'MicroPython', 'I2S', 'PWM', 'MPU6050',
        'DS18B20', '温度', '视觉', '机械臂'
    ];
    
    for (const keyword of keywords) {
        if (name.includes(keyword.toLowerCase()) && !tags.includes(keyword)) {
            tags.push(keyword);
        }
    }
    
    // 限制标签数量
    return tags.slice(0, 5);
}

/**
 * 生成文章内容
 * @param {object} projectInfo - 项目信息
 * @param {string} projectType - 项目类型
 * @returns {string} - Markdown 文章内容
 */
function generateArticle(projectInfo, projectType) {
    const title = generateTitle(projectInfo.name, projectType);
    const summary = generateSummary(projectInfo, projectType);
    const codeFiles = projectInfo.codeFiles.slice(0, 3); // 最多取3个代码文件
    
    let article = `# ${title}

## 项目简介

${summary}

## 功能特性

- ✅ **核心功能**：实现${projectType}相关功能
- ✅ **易于上手**：提供完整的代码和详细说明
- ✅ **扩展性强**：可根据需求进行修改和扩展

## 所需材料

### 硬件
- **${projectType}开发板**
- **相关传感器/模块**（根据项目需求）
- **杜邦线** 若干

### 软件
- **开发环境**（如 Arduino IDE、Keil 等）
- **相关库文件**

## 项目结构

\`\`\`
${projectInfo.name}/
${projectInfo.files.slice(0, 10).map(f => `├── ${f}`).join('\n')}
${projectInfo.files.length > 10 ? `└── ... (${projectInfo.files.length - 10} 个其他文件)` : ''}
\`\`\`

`;

    // 添加代码示例
    if (codeFiles.length > 0) {
        article += `## 核心代码

`;
        for (const codeFile of codeFiles) {
            const content = readCodeFile(path.join(projectInfo.path, codeFile));
            const ext = path.extname(codeFile).substring(1);
            article += `### ${codeFile}

\`\`\`${ext}
${content}
\`\`\`

`;
        }
    }

    // 添加使用说明
    article += `## 使用方法

### 1. 硬件连接

根据项目需求连接相应的硬件模块。

### 2. 软件准备

1. 安装开发环境
2. 导入必要的库文件
3. 配置相关参数

### 3. 编译上传

1. 打开项目代码
2. 选择正确的开发板和端口
3. 编译并上传程序

## 扩展功能

1. **功能扩展**：根据需求添加新的功能模块
2. **性能优化**：优化代码提高运行效率
3. **界面美化**：改进用户交互界面

## 常见问题

### 1. 编译错误
- 检查库文件是否正确安装
- 确认开发板型号选择正确

### 2. 硬件无响应
- 检查接线是否正确
- 确认电源供应稳定

## 参考文档

- [官方文档](https://docs.espressif.com/)
- [相关教程](https://www.arduino.cc/)

---

**注意**：使用本项目时请注意安全，确保硬件连接正确。
`;

    return article;
}

/**
 * 获取下一个文章ID
 * @returns {string} - 如 "article20"
 */
function getNextArticleId() {
    const loaderPath = path.join(CONFIG.projectRoot, CONFIG.loaderFile);
    const content = fs.readFileSync(loaderPath, 'utf-8');
    
    // 查找所有 articleX 的 ID
    const matches = content.match(/'article(\d+)'/g);
    if (!matches) return 'article1';
    
    let maxId = 0;
    for (const match of matches) {
        const id = parseInt(match.match(/\d+/)[0]);
        if (id > maxId) maxId = id;
    }
    
    return `article${maxId + 1}`;
}

/**
 * 更新 markdown-loader.js
 * @param {string} articleId - 文章ID
 * @param {object} meta - 文章元数据
 */
function updateLoader(articleId, meta) {
    const loaderPath = path.join(CONFIG.projectRoot, CONFIG.loaderFile);
    let content = fs.readFileSync(loaderPath, 'utf-8');
    
    // 生成新的元数据行
    const tagsStr = meta.tags.map(t => `'${t}'`).join(', ');
    const newLine = `    '${articleId}': { title: '${meta.title}', author: '${meta.author}', date: '${meta.date}', category: '${meta.category}', comments: '${meta.comments}', tags: [${tagsStr}], summary: '${meta.summary}' }`;
    
    // 在最后一个文章条目后添加
    const lastEntryRegex = /('article\d+':\s*\{[^}]+\}\s*,?\s*)(\n\};)/;
    if (lastEntryRegex.test(content)) {
        content = content.replace(lastEntryRegex, `$1,\n${newLine}$2`);
    } else {
        // 如果没有匹配到，在 articlesMeta 对象末尾添加
        content = content.replace(/};\s*$/, `,\n${newLine}\n};`);
    }
    
    fs.writeFileSync(loaderPath, content);
    console.log(`✅ 已更新 ${CONFIG.loaderFile}`);
}

/**
 * 保存文章文件
 * @param {string} articleId - 文章ID
 * @param {string} content - 文章内容
 */
function saveArticle(articleId, content) {
    const articlePath = path.join(CONFIG.projectRoot, CONFIG.contentDir, `${articleId}.md`);
    fs.writeFileSync(articlePath, content);
    console.log(`✅ 已保存文章: ${articlePath}`);
}

// ==================== 主程序 ====================

function main() {
    console.log('🚀 FlowerSea\'s Blog - 自动化文章生成工具\n');
    
    // 扫描项目目录
    const projectsDir = path.join(CONFIG.projectRoot, CONFIG.projectsDir);
    
    if (!fs.existsSync(projectsDir)) {
        console.error(`❌ 项目目录不存在: ${projectsDir}`);
        return;
    }
    
    const items = fs.readdirSync(projectsDir);
    const projects = items.filter(item => {
        const itemPath = path.join(projectsDir, item);
        return fs.statSync(itemPath).isDirectory();
    });
    
    console.log(`📁 发现 ${projects.length} 个项目:\n`);
    
    // 显示项目列表
    projects.forEach((proj, index) => {
        console.log(`${index + 1}. ${proj}`);
    });
    
    console.log('\n💡 使用说明:');
    console.log('   1. 将项目文件夹放入 "要上传到网页的文章" 目录');
    console.log('   2. 运行此脚本');
    console.log('   3. 根据提示选择要生成的项目');
    console.log('   4. 脚本会自动生成文章并更新配置\n');
    
    // 这里可以添加交互式选择
    // 为了简化，这里演示自动处理所有项目
    
    console.log('🔧 开始自动处理...\n');
    
    let processedCount = 0;
    
    for (const projectName of projects) {
        const projectPath = path.join(projectsDir, projectName);
        
        console.log(`📂 正在处理: ${projectName}`);
        
        try {
            // 扫描项目
            const projectInfo = scanProject(projectPath);
            console.log(`   发现 ${projectInfo.fileCount} 个文件`);
            
            // 识别项目类型
            const projectType = detectProjectType(projectInfo);
            console.log(`   项目类型: ${projectType}`);
            
            // 生成文章
            const articleContent = generateArticle(projectInfo, projectType);
            
            // 生成元数据
            const articleId = getNextArticleId();
            const meta = {
                title: generateTitle(projectName, projectType),
                author: CONFIG.author,
                date: new Date().toLocaleDateString('zh-CN', { year: 'numeric', month: 'long', day: 'numeric' }).replace(/\//g, '年').replace(/$/, '日').replace(/年(\d+)年/, '年$1月').replace(/月(\d+)日/, '月$1日'),
                category: generateCategory(projectType, projectName),
                comments: '0 条',
                tags: generateTags(projectName, projectType),
                summary: generateSummary(projectInfo, projectType)
            };
            
            // 保存文章
            saveArticle(articleId, articleContent);
            
            // 更新配置
            updateLoader(articleId, meta);
            
            processedCount++;
            console.log(`   ✅ 已生成文章: ${articleId}\n`);
            
        } catch (err) {
            console.error(`   ❌ 处理失败: ${err.message}\n`);
        }
    }
    
    console.log(`🎉 处理完成! 共生成 ${processedCount} 篇文章`);
    console.log('\n📋 下一步操作:');
    console.log('   1. 检查生成的文章质量');
    console.log('   2. 上传 content/ 目录下的新文章到托管平台');
    console.log('   3. 上传更新后的 js/markdown-loader.js');
    console.log('   4. 强制刷新浏览器查看效果');
}

// 运行主程序
if (require.main === module) {
    main();
}

// 导出函数供外部使用
module.exports = {
    scanProject,
    detectProjectType,
    generateArticle,
    generateCategory,
    updateLoader,
    getNextArticleId
};
