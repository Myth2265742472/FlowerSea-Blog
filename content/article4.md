Git 是现代软件开发中不可或缺的版本控制工具。掌握 Git 的使用技巧不仅能提高开发效率，还能更好地管理项目代码。本文将介绍一些实用的 Git 技巧，帮助你更高效地管理代码版本。

## Git 基础概念

### 1. 三个区域

理解 Git 的三个工作区域是掌握 Git 的基础：

```text
工作区 (Working Directory)
  ↓ git add
暂存区 (Staging Area / Index)
  ↓ git commit
本地仓库 (Local Repository)
  ↓ git push
远程仓库 (Remote Repository)
```

### 2. 常用命令速查

```bash
# 初始化仓库
git init

# 克隆远程仓库
git clone <url>

# 查看状态
git status

# 添加文件到暂存区
git add .
git add <file>

# 提交更改
git commit -m "提交信息"

# 查看提交历史
git log --oneline

# 查看远程仓库
git remote -v
```

## 实用技巧

### 1. 优雅的提交信息

使用约定式提交（Conventional Commits）规范：

```bash
# 格式：<type>(<scope>): <subject>
feat: 新增用户登录功能
fix: 修复导航栏显示问题
docs: 更新 README 文档
style: 优化代码格式
refactor: 重构用户模块
test: 添加单元测试
chore: 更新依赖包
```

### 2. 暂存当前工作

当需要切换分支但当前工作未完成时：

```bash
# 暂存当前更改
git stash

# 查看暂存列表
git stash list

# 恢复暂存
git stash pop

# 应用指定暂存
git stash apply stash@{0}

# 删除暂存
git stash drop
```

### 3. 撤销操作

```bash
# 撤销工作区修改
git checkout -- <file>
git restore <file>

# 撤销暂存区修改
git reset HEAD <file>
git restore --staged <file>

# 撤销提交（保留更改）
git reset --soft HEAD~1

# 撤销提交（丢弃更改）
git reset --hard HEAD~1

# 修改最后一次提交
git commit --amend
```

### 4. 分支管理

```bash
# 创建并切换分支
git checkout -b feature/xxx

# 切换分支
git checkout develop

# 查看所有分支
git branch -a

# 删除本地分支
git branch -d feature/xxx

# 删除远程分支
git push origin --delete feature/xxx

# 重命名分支
git branch -m old-name new-name
```

### 5. 合并策略

```bash
# 普通合并（产生合并提交）
git merge feature-branch

# 变基合并（保持线性历史）
git rebase main

# 合并特定提交
git cherry-pick <commit-hash>

# 暂时合并（不提交）
git merge --squash feature-branch
```

### 6. 解决冲突

当遇到合并冲突时：

```bash
# 标记冲突文件
# <<<<<<< HEAD
# 当前分支的内容
# =======
# 被合并分支的内容
# >>>>>>> feature-branch

# 手动解决冲突后

# 标记为已解决
git add <file>

# 继续合并
git commit
```

## 高级技巧

### 1. Git别名

常用别名配置：

```bash
# 配置别名
git config --global alias.st status
git config --global alias.co checkout
git config --global alias.br branch
git config --global alias.ci commit
git config --global alias.unstage 'reset HEAD --'
git config --global alias.last 'log -1 HEAD'
git config --global alias.visual 'log --graph --oneline --all --decorate'

# 使用示例
git st  # 等同于 git status
git co main  # 等同于 git checkout main
```

### 2. 查找引入Bug的提交

```bash
# 二分查找
git bisect start
git bisect bad  # 标记当前版本有问题
git bisect good <good-commit-hash>  # 标记已知好的版本
# Git会自动切换到中间版本，测试后标记
git bisect good  # 或 git bisect bad
# 重复直到找到问题提交
git bisect reset  # 结束二分查找
```

### 3. 交互式变基

修改历史提交：

```bash
# 交互式变基最近3次提交
git rebase -i HEAD~3

# 命令说明
pick <commit-hash> # 使用该提交
reword <commit-hash> # 修改提交信息
edit <commit-hash> # 编辑提交
squash <commit-hash> # 合并到前一个提交
drop <commit-hash> # 丢弃该提交
```

### 4. Cherry Pick

将其他分支的提交应用到当前分支：

```bash
# 应用单个提交
git cherry-pick <commit-hash>

# 应用多个提交
git cherry-pick <commit1> <commit2>

# 应用范围提交
git cherry-pick <start-commit>..<end-commit>
```

### 5. 搜索代码

```bash
# 搜索代码内容
git grep "functionName"

# 搜索提交信息
git log --grep "fix bug"

# 搜索修改某文件的提交
git log -- <file-path>
```

## 远程协作

### 1. 拉取代码

```bash
# 拉取并合并
git pull

# 拉取但不合并
git fetch

# 拉取特定分支
git fetch origin feature-branch
```

### 2. 推送代码

```bash
# 推送当前分支
git push

# 推送到指定远程和分支
git push origin feature-branch

# 设置上游分支
git push -u origin feature-branch

# 强制推送（谨慎使用）
git push --force
```

### 3. 查看远程信息

```bash
# 查看远程仓库
git remote -v

# 查看远程分支
git branch -r

# 查看远程仓库详情
git remote show origin
```

## 实用工具

### 1. .gitignore 配置

```gitignore
# 依赖
node_modules/
bower_components/

# 日志
npm-debug.log
yarn-error.log

# 构建产物
dist/
build/
*.min.js

# 编辑器
.vscode/
.idea/
*.swp
*.swo

# 环境变量
.env
.env.local

# 操作系统
.DS_Store
Thumbs.db
```

### 2. Git Hooks

自动执行任务：

```bash
# 提交前检查代码格式
# .git/hooks/pre-commit
#!/bin/sh
npm run lint
npm test

# 推送前运行测试
# .git/hooks/pre-push
#!/bin/sh
npm run test:ci
```

### 3. Git Flow 工作流

```bash
# 功能开发
git checkout -b feature/user-login

# 完成后合并到develop
git checkout develop
git merge feature/user-login

# 发布版本
git checkout -b release/v1.0.0
# 测试和修复后
git checkout main
git merge release/v1.0.0

# 紧急修复
git checkout -b hotfix/critical-bug
# 修复后合并到main和develop
```

## 性能优化

### 1. 浅克隆

```bash
# 只克隆最新提交，节省时间和空间
git clone --depth 1 <url>

# 浅克隆后获取完整历史
git fetch --unshallow
```

### 2. 垃圾回收

```bash
# 清理无用文件
git gc

# 激进清理（谨慎使用）
git gc --prune=now
```

## 最佳实践

1. **频繁提交**：小步快跑，每次提交都是一个可工作状态
2. **清晰信息**：提交信息要说明"做了什么"和"为什么"
3. **分支策略**：使用分支管理不同功能的开发
4. **代码审查**：合并前进行代码审查
5. **定期拉取**：保持本地代码与远程同步
6. **备份重要分支**：在重要里程碑打标签

## 总结

Git 是一个功能强大的工具，掌握这些技巧可以大大提高开发效率。建议在实际项目中多练习，逐渐形成自己的工作流程。

记住：Git 不可怕，多实践就会熟练！
