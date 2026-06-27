# 🗺️ 校园探索与智能导航模拟系统

<div align="center">

![Qt](https://img.shields.io/badge/Qt-6.5+-41CD52?style=for-the-badge&logo=qt&logoColor=white)
![C++](https://img.shields.io/badge/C++-17-00599C?style=for-the-badge&logo=c%2B%2B&logoColor=white)
![CMake](https://img.shields.io/badge/CMake-3.16+-064F8C?style=for-the-badge&logo=cmake&logoColor=white)
![SQLite](https://img.shields.io/badge/SQLite-3-003B57?style=for-the-badge&logo=sqlite&logoColor=white)
![Platform](https://img.shields.io/badge/Platform-Windows-0078D6?style=for-the-badge&logo=windows&logoColor=white)

**基于 Qt 6 的 2.5D 校园探索与智能导航模拟系统**

*支持地图漫游 · Dijkstra 路径规划 · 多终点 TSP 导航 · 昼夜切换 · NPC 对话*

</div>

---

## 📖 项目简介

本项目是一个基于 **Qt 6 Graphics View Framework** 开发的 2.5D 校园探索与智能导航模拟系统，面向《C++程序设计》课程设计。

系统以真实校园为蓝本，构建了包含 **15+ 个建筑物** 和完整路网的校园地图。用户可以操控角色在校园中自由漫游，通过 **Dijkstra 最短路径算法** 规划路线，体验角色自动沿路径行走的导航过程，并支持多终点 TSP 近似规划、昼夜切换、NPC 对话等扩展功能。

---

## ✨ 功能特性

### 🔵 基础功能

| 功能 | 描述 |
|------|------|
| 🗺️ **校园地图浏览** | 2.5D 俯视视角，支持鼠标滚轮缩放、拖拽平移 |
| 🏛️ **建筑信息查询** | 点击建筑物弹出详情面板，展示名称、简介等信息 |
| 🔍 **建筑搜索** | 搜索框输入名称，自动补全并快速定位目标建筑 |
| 📍 **最短路径规划** | Dijkstra 算法计算两点间最短路径，高亮显示路线 |
| 🚶 **角色手动漫游** | WASD 控制角色移动，四方向精灵动画，步行/奔跑切换 |
| 🚀 **角色自动导航** | 选定起终点后，角色沿规划路径平滑自动行走 |
| 🗄️ **建筑信息管理** | 管理员模式支持通过 SQLite 对建筑信息进行增删改查 |

### 🟡 扩展功能

| 功能 | 描述 |
|------|------|
| 🌙 **昼夜切换** | 一键切换日间/夜间地图风格，夜间模式叠加深色滤镜 |
| 🗺️ **多终点路径规划** | 支持添加多个途经点，TSP 贪心近邻算法规划最优访问顺序 |
| 🧑‍🏫 **NPC 系统** | 角色靠近 NPC 自动触发对话，配备冷却机制，NPC 可点击交互 |

---

## 🏗️ 项目架构

```
CampusNavigation/
├── main.cpp                  # 程序入口，完成数据加载与窗口启动
├── CMakeLists.txt            # CMake 构建配置
│
├── core/                     # 核心业务层（与 UI 完全解耦）
│   ├── map/                  # 地图数据模型
│   │   ├── Building.h/cpp    #   建筑结构体（ID、名称、坐标、入口节点）
│   │   └── CampusMap.h/cpp   #   校园地图管理（从 JSON 加载全图数据）
│   ├── graph/                # 图结构
│   │   └── Graph.h/cpp       #   邻接表实现（Node + Edge）
│   └── pathfinding/          # 路径算法
│       └── Dijkstra.h/cpp    #   Pathfinder 类（Dijkstra + TSP 贪心）
│
├── graphics/                 # 可视化层（Qt Graphics View）
│   ├── MapView.h/cpp         #   地图视图（渲染、输入、游戏主循环）
│   ├── CharacterItem.h/cpp   #   玩家角色图元（精灵动画、自动导航）
│   └── NpcItem.h/cpp         #   NPC 图元（触发检测、冷却管理）
│
├── controller/               # 控制层（业务流程协调）
│   └── NavigationController  #   导航状态机（起点→多终点→路径计算→自动行走）
│
├── database/                 # 数据持久化层
│   └── DatabaseManager.h/cpp #   SQLite DAO（建筑信息 CRUD）
│
├── ui/                       # 界面层
│   ├── MainWindow.h/cpp      #   主窗口（面板布局、信号槽连接）
│   └── DialogWidget.h/cpp    #   NPC 对话框组件
│
└── resources/                # 静态资源（通过 Qt 资源系统嵌入程序）
    ├── campus_map.png        #   校园背景地图图片
    ├── character.png         #   玩家精灵图（4方向行走动画帧）
    ├── npc.png               #   NPC 图片
    ├── map_data_fuben.json   #   校园路网与建筑数据（JSON 格式）
    └── resources.qrc         #   Qt 资源描述文件
```

### 架构分层说明

```
┌─────────────────────────────────────────┐
│           ui / MainWindow               │  ← 界面层：只负责显示和用户交互
├─────────────────────────────────────────┤
│       controller / NavController        │  ← 控制层：协调路径规划与角色行走
├──────────────┬──────────────────────────┤
│  graphics /  │     database /           │  ← 执行层：渲染 & 数据持久化
│  MapView     │     DatabaseManager      │
├──────────────┴──────────────────────────┤
│   core / CampusMap + Graph + Pathfinder │  ← 算法层：图结构与路径算法（纯逻辑）
└─────────────────────────────────────────┘
```

---

## 🔧 编译环境要求

| 依赖 | 最低版本 | 说明 |
|------|----------|------|
| Qt | 6.5 LTS+ | 需包含 `Widgets`、`Sql` 模块 |
| CMake | 3.16+ | 构建系统 |
| C++ 编译器 | C++17 | MSVC 2022 或 MinGW-w64 |
| 操作系统 | Windows 10+ | 当前仅支持 Windows 平台 |

> **推荐安装方式**：从 [Qt 官网](https://www.qt.io/download) 下载 Qt 6.5+ 在线安装器，选择 `MSVC 2022 64-bit` 或 `MinGW 13.1.0 64-bit` 套件。

---

## 🚀 一键编译运行

### 方法一：命令行（推荐）

```bash
# 1. 克隆仓库
git clone <仓库地址>
cd CampusNavigation

# 2. 配置 CMake（请将 Qt6_DIR 替换为你的实际路径）
cmake -S . -B build -DCMAKE_PREFIX_PATH="C:/Qt/6.5.3/msvc2022_64"

# 3. 编译
cmake --build build --config Release

# 4. 运行
./build/Release/CampusNavigator.exe
```

> **注意**：首次运行会在程序目录自动创建 `campus_data.db` 数据库文件，无需手动配置。

### 方法二：Qt Creator（推荐新手使用）

1. 打开 **Qt Creator**
2. 选择菜单 `文件` → `打开文件或项目`
3. 选择项目根目录下的 **`CMakeLists.txt`**
4. 在弹出的 Kit 配置中，选择 **Qt 6.5+ 对应的套件**（MSVC 或 MinGW 均可）
5. 等待 CMake 自动配置完成
6. 点击左下角 ▶ **运行** 按钮（或按 `Ctrl+R`）

---

## 🎮 操作说明

### 基本操控

| 操作 | 效果 |
|------|------|
| `鼠标滚轮` | 缩放地图 |
| `鼠标左键拖拽` | 平移地图视图 |
| `W / A / S / D` | 控制角色上/左/下/右移动 |
| `点击建筑物` | 查看建筑详细信息 |

### 导航流程

1. **搜索建筑**：在右侧搜索框输入建筑名称（支持自动补全），按 Enter 定位
2. **设置起点**：点击目标建筑 → 点击面板中的 **"设为起点"** 按钮
3. **添加途经点**：继续点击其他建筑 → 点击 **"添加途经点"**（可添加多个）
4. **开始导航**：点击 **"开始导航"** 按钮，角色自动沿规划路径行走
5. **清空路线**：点击 **"清空"** 按钮重置所有选点

### 其他功能

| 操作 | 效果 |
|------|------|
| `步行 / 奔跑` 单选框 | 切换角色移动速度 |
| `昼夜模式` 复选框 | 切换地图日间/夜间风格 |
| 靠近 NPC 角色 | 自动触发 NPC 对话（或点击 NPC） |
| **编辑信息** 按钮 | 进入管理员模式，修改建筑简介 |

---

## 🧮 核心算法说明

### Dijkstra 最短路径

- **数据结构**：邻接表（`std::unordered_map<int, Node>`）+ 最小优先队列
- **时间复杂度**：O((V + E) log V)
- **流程**：
  1. `CampusMap` 从 JSON 文件中加载所有节点（Node）和边（Edge）
  2. 建筑物通过 `entrance_node_id` 桥接到路网（建筑中心与路径节点分离设计）
  3. `NavigationController` 调用 `Pathfinder::findShortestPath(startNodeId, endNodeId)`
  4. 返回节点 ID 序列，`MapView::drawPath()` 将其转换为高亮折线显示

### 多终点 TSP 贪心近邻

- **适用场景**：用户添加多个途经点时
- **算法思路**：从起点出发，每一步选取 Dijkstra 距离最近的未访问目标，依次连接
- **实现方法**：`Pathfinder::findTSPPath(startId, destIds, outVisitOrder)`
- **结果**：将多段最短路径拼接为完整路径，并在界面显示建议访问顺序

---

## 📁 资源文件说明

| 文件 | 说明 | 是否需要手动配置 |
|------|------|-----------------|
| `resources/map_data_fuben.json` | 校园路网与建筑数据 | ❌ 已通过 Qt 资源系统（`.qrc`）嵌入程序 |
| `resources/campus_map.png` | 校园背景地图 | ❌ 已嵌入程序 |
| `resources/character.png` | 角色精灵图 | ❌ 已嵌入程序 |
| `campus_data.db` | SQLite 建筑信息数据库 | ❌ 程序首次运行时自动生成 |

> 所有资源均通过 `resources/resources.qrc` 以 `:/` 前缀嵌入可执行文件，**无需在运行时单独部署资源文件**。

---

## 📄 开源协议

本项目仅用于课程设计学习目的，代码遵循 [MIT License](LICENSE)。
