# Arcane Campus Online 🏰

> A multiplayer online text RPG social platform built with C++20, MySQL, and Qt6.
>
> 基于 C++20 + MySQL + Qt6 构建的多人在线文字 RPG 社交平台。

---

## 项目简介 / About

### 中文

**Arcane Campus Online** 是一个以魔法校园为背景的多人在线文字 RPG 社交平台。

项目从一个轻量级 Socket 聊天室开始，逐步演进为支持多场景、多角色、实时互动的虚拟校园世界。玩家以角色身份进入世界，在礼堂、图书馆、天文塔、公共休息室等场景中进行聊天、社交、探索与互动。

除了构建一个完整的文字世界之外，本项目也是一个持续迭代的 Linux 网络编程与桌面应用实践平台，用于探索现代 C++ 后端开发中的分层架构、数据库设计、连接池、以及客户端开发技术。

> 本项目为非商业同人作品。
> Harry Potter © J.K. Rowling / Warner Bros. 版权所有。

### English

**Arcane Campus Online** is a multiplayer online text RPG social platform inspired by the magical world of Harry Potter.

Starting from a lightweight socket chatroom, the project gradually evolves into a multi-scene, multi-character, real-time interactive virtual campus. Players enter the world as characters and interact with others in locations such as the Great Hall, Library, Astronomy Tower, Dormitories, and more.

The platform aims to provide an immersive text-based social experience while serving as a practical playground for modern C++ backend architecture, database engineering, and desktop client development.

> Fan Project — Non-commercial use only.
> Harry Potter © J.K. Rowling / Warner Bros. All rights reserved.

---

## ✨ 已实现功能 / Implemented Features

### 🎮 客户端 (Qt6 Desktop Client)

| 功能 | 说明 |
|------|------|
| 🪄 登录系统 | 选择学院、定制魔杖、真实数据库验证 |
| 🏠 主界面 | 8 个功能 Widget + 暗色魔法主题 |
| 💬 聊天事件面板 | 羊皮纸风格，支持多频道（Location / World / Party）|
| 🎒 背包系统 | **真实数据库对接**（InventoryDAO）|
| 🗺️ 活点地图 | **真实数据库对接**（InventoryDAO + LocationDAO）|
| 📋 任务系统 | **真实数据库对接**（QuestDAO）|
| 👤 角色状态面板 | 显示学院、等级、位置、状态 |
| 🕐 校园时钟 | 实时刷新 |
| 🏆 学院积分排行 | 4 个学院实时排名 |
| 🤖 NPC AI 对话 | DeepSeek API 驱动 |

### 🗄️ 后端数据层（MySQL）

| 模块 | 状态 |
|------|------|
| 真实 MySQL 驱动（MySqlDatabaseDriver） | ✅ 动态加载 libmysql.dll |
| 数据库连接池（RAII） | ✅ |
| INI 配置加载（DatabaseConfigLoader） | ✅ |
| 11 张表完整 Schema（含 player_locations） | ✅ |
| 种子数据（3 个测试用户 + 4 个角色 + 活点地图道具） | ✅ |

---

## 🛠️ 技术栈 / Tech Stack

| 层次 / Layer | 技术 / Technology |
|---|---|
| 开发语言 / Language | **C++20** |
| 操作系统 / OS | Windows / Linux（开发用 Windows，生产用 Ubuntu） |
| 数据库 / Database | **MySQL 8.0**（InnoDB, utf8mb4）|
| 数据库抽象 | **Strategy Pattern · Connection Pool (RAII) · DAO Pattern** |
| 服务端分层 | Controller → Service → DAO → DO / DTO / VO |
| 客户端 / Client | **Qt6 Widgets** · QSS 主题（墨绿炭黑·Quiet Library Theme） |
| 缓存系统 / Cache | Redis（规划中） |
| 构建工具 / Build | CMake + CTest |

---

## 🏗️ 系统架构 / Architecture

```
                  Qt6 Desktop Client
              (Widgets + QSS + Signals/Slots)
                          │
                Qt Signal/Slot (in-process)
                          │
                CampusController
                (16 slots / 6 signals)
                          │
        ┌─────────────────┼─────────────────┐
        │                 │                 │
   SessionService    ChatService      InventoryService
   CampusService     MapService       QuestService
   SocialService
        │                 │                 │
        └─────────────────┼─────────────────┘
                          │
              DAO 层 (shared_ptr<DBConnection>)
        ┌────────┬────────┬────────┬────────┐
        │        │        │        │        │
     UserDAO  ItemDAO  QuestDAO  RoomDAO  FriendDAO
   CharacterDAO InventoryDAO LocationDAO MessageDAO
                          │
                   DBConnectionPool
                          │
                  MySqlDatabaseDriver
                          │
                       MySQL
```

### 服务端分层 / Server Layering

| 层 | 命名空间 | 目录 | 职责 |
|---|---------|------|------|
| **Controller** | `arcane::application::controller` | `src/application/controller/` | 信号槽路由，参数绑定 |
| **Service** | `arcane::application::service` | `src/application/service/` | 核心业务逻辑编排，DAO 注入 |
| **DAO** | `arcane::database` | `database/include/, database/src/` | 数据库 CRUD，参数化查询 |
| **DO** | `arcane::application::do_model` | `src/application/do/` | 数据库表映射对象 |
| **DTO** | `arcane::application::dto` | `src/application/dto/` | 请求参数封装 |
| **VO** | `arcane::application::vo` | `src/application/vo/` | 接口返回视图对象 |
| **Router** | `arcane::application::router` | `src/application/router/` | 未来 TCP 网络层的命令路由预留 |

---

## 📂 项目结构 / Project Structure

```
HogwartsOnline/
├── CMakeLists.txt                      # 顶层构建脚本
│
├── database/                           # ── 数据层（独立静态库）──
│   ├── include/                        # DAO + 驱动接口（14 个 hpp）
│   │   ├── IDatabaseDriver.hpp
│   │   ├── DBConnection.hpp
│   │   ├── DBConnectionPool.hpp
│   │   ├── DatabaseConfigLoader.hpp
│   │   ├── RecordMapper.hpp
│   │   └── User/Character/Room/Friend/Message/NPC/
│   │       Inventory/Quest/LocationDAO.hpp
│   │
│   ├── src/                            # DAO + 驱动实现（全部真实 SQL）
│   │   ├── MySqlDatabaseDriver.cpp     # MySQL C API（动态加载 libmysql.dll）
│   │   ├── NoopDatabaseDriver.cpp
│   │   └── 9 个 DAO.cpp
│   │
│   ├── sql/                            # 数据库脚本
│   │   ├── init_mysql.sql              # 11 张表建表
│   │   ├── seed_mysql.sql              # 种子数据（用户、角色、活点地图道具等）
│   │   └── migrate_v1.sql
│   │
│   ├── scripts/                        # PowerShell 脚本
│   │   ├── import_mysql.ps1
│   │   └── verify_mysql.ps1
│   │
│   ├── seed_redis_commands.txt
│   └── config/database.ini.example     # 数据库配置模板
│
├── src/                                # ── 应用层（Qt6 客户端）──
│   ├── main.cpp                        # 入口：数据库接入 + 信号槽连接
│   │
│   ├── application/
│   │   ├── do/CampusDO.hpp             # 4 个 DO
│   │   ├── dto/CampusDTO.hpp           # 8 个 DTO
│   │   ├── vo/CampusVO.hpp             # 8 个 VO + 1 别名
│   │   ├── service/
│   │   │   ├── SessionService          # ✅ 接 UserDAO+CharacterDAO（真实登录）
│   │   │   ├── ChatService             # ✅ 接 MessageDAO（聊天持久化）
│   │   │   ├── InventoryService        # ✅ 接 InventoryDAO
│   │   │   ├── QuestService            # ✅ 接 QuestDAO
│   │   │   ├── MapService              # ✅ 接 InventoryDAO+LocationDAO（活点地图）
│   │   │   ├── CampusService           # 🔧 硬编码（待接 RoomDAO）
│   │   │   └── SocialService           # 🔧 硬编码（待接 FriendDAO）
│   │   ├── controller/
│   │   │   └── CampusController        # 信号槽枢纽（16 slots + 6 signals）
│   │   └── router/
│   │       └── ClientCommandRouter     # ✅ 未来 TCP 网络层命令路由预留
│   │
│   ├── ui/
│   │   ├── LoginWindow.{h,cpp}         # 登录窗口
│   │   ├── MainWindow.{h,cpp}          # 主窗口
│   │   └── widgets/                    # 8 个功能 Widget
│   │       ├── CampusTimeWidget
│   │       ├── CharacterStatusWidget
│   │       ├── ChatEventWidget
│   │       ├── CourseScheduleWidget
│   │       ├── HouseRankingWidget
│   │       ├── InventoryWidget
│   │       ├── LocationActionBar
│   │       └── MemberListWidget
│   │
│   └── resources/
│       ├── resources.qrc
│       └── qss/dark_wood.qss           # 墨绿炭黑主题（Quiet Library Theme）
│
├── tests/                              # 单元测试
│   └── database/
│       ├── DatabaseSmokeTest.cpp       # 冒烟测试（无需数据库）
│       └── MySqlIntegrationTest.cpp    # MySQL 集成测试
│
└── README.md
```

---

## 📊 当前进度 / Progress

### ✅ 已完成 / Completed

- [x] UDP Chatroom Demo
- [x] Thread Pool
- [x] Producer-Consumer Architecture
- [x] Modular Project Structure
- [x] **MySQL Integration** (真实驱动 + 连接池 + 9 个 DAO)
- [x] **Qt Desktop Client** (UI 完整 + 暗色魔法主题)
- [x] **真实登录系统** (数据库验证用户名+学院)
- [x] **背包系统** (真实数据库 CRUD)
- [x] **活点地图** (道具校验 + 在线玩家位置查询)
- [x] **任务系统** (任务列表 + 进度更新)
- [x] **聊天持久化** (MessageDAO 写入)
- [x] **AI NPC 系统** (DeepSeek API 集成)

### 🚧 进行中 / In Progress

- [ ] TCP Persistent Connection (ClientCommandRouter 已预留)
- [ ] Epoll-based High Concurrency Refactor

### 📋 规划中 / Planned

- [ ] Redis Integration（缓存层）
- [ ] Room System 完整实现（CampusService 接 RoomDAO）
- [ ] Friend System 完整实现（SocialService 接 FriendDAO）
- [ ] Achievement System
- [ ] Persistent world data storage（聊天历史归档）

---

## 🚀 快速开始 / Quick Start

### 环境要求 / Prerequisites

| 依赖 | 版本 |
|------|------|
| CMake | 3.20+ |
| Qt6 | 6.5+ (Widgets 组件) |
| MySQL | 8.0+ |
| MySQL C API | 8.0+ (libmysql.dll / libmysqlclient) |
| 编译器 | MSVC 2022 / MinGW / GCC 11+ |
| C++ 标准 | C++20 |

### 1️⃣ 初始化数据库 / Initialize Database

```powershell
# Windows PowerShell
cd database/scripts
.\import_mysql.ps1
```

或者手动在 Navicat / MySQL CLI 中执行：

```bash
mysql -u root -p < database/sql/init_mysql.sql
mysql -u root -p < database/sql/seed_mysql.sql
```

### 2️⃣ 配置数据库连接 / Configure Database

复制配置模板并修改密码：

```bash
cp database/config/database.ini.example database.ini
```

编辑 `database.ini`：

```ini
driver=mysql
host=127.0.0.1
port=3306
database=arcane_campus_online
username=root
password=******           ; 修改为你的 MySQL 密码
character_set=utf8mb4
client_library_path=D:/mysql-8.0.25-winx64/bin/libmysql.dll
```

> **或者**设置环境变量 `ARCANE_DB_PASSWORD`，密码不会被提交到 git。
> **或者**设置环境变量 `ARCANE_DB_CONFIG` 指定配置文件路径。

### 3️⃣ 构建项目 / Build

```bash
mkdir build && cd build
cmake .. -G "MinGW Makefiles"   # 或者 "Visual Studio 17 2022"
cmake --build .
```

### 4️⃣ 运行 / Run

```bash
./HogwartsOnline.exe           # Windows
./HogwartsOnline               # Linux
```

---

## 🧪 测试 / Testing

```bash
cd build
ctest --output-on-failure
```

测试类型：
- **DatabaseSmokeTest** — 冒烟测试，无需数据库
- **MySqlIntegrationTest** — MySQL 集成测试（需先初始化数据库）

---

## 🧪 测试账号 / Test Accounts

种子数据中包含以下测试用户：

| 用户名 | 学院 | 说明 |
|--------|------|------|
| `raven_test` | Ravenclaw | 拥有活点地图道具，可测试 Marauder's Map |
| `gryffin_test` | Gryffindor | - |
| `slyther_test` | Slytherin | - |

---

## 🎯 项目目标 / Goals

- Practice modern C++ backend engineering and Linux network programming.
- Explore high-concurrency server design patterns (Epoll + Reactor planned).
- Build a long-term portfolio project demonstrating system architecture skills.
- Accumulate experience in database engineering, connection pooling, and desktop application development.

---

## 👤 作者 / Author



CS Student · Backend & Systems Enthusiast

Currently learning and experimenting with:

* Modern C++ / C++20
* Linux Network Programming
* Concurrency & Server Architecture
* Database Fundamentals
* Qt Desktop Development

Mostly learning by building small projects and messing around with ideas I find interesting.


---

## 📄 License

MIT License — See [LICENSE](./LICENSE) for details.

> Fan Project · Non-commercial use only.
> Harry Potter © J.K. Rowling / Warner Bros.
