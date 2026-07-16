# 🖥️ Server 服务端逻辑层

本目录包含 Arcane Campus Online 的服务端业务逻辑代码，采用 **5层分层架构** 设计。

> ⚠️ 当前状态：预留代码，尚未集成到 CMakeLists.txt 构建中。

## 分层架构

```
 Controller (路由控制层)
     │
     ▼
  Service (业务逻辑层)
     │
     ▼
    DAO (数据访问层)
     │
     ▼
  DO / VO / Query (数据模型)
```

## 目录结构

```
server/
├── Controller/                 # 路由控制层 - 处理 HTTP 请求
│   ├── InventoryController     # GET /inventory/list?characterId=&page=&pageSize=
│   └── MapController           # GET /map/use-marauders-map?characterId=
│
├── Service/                    # 业务逻辑层 - 核心业务编排
│   ├── InventoryService        # 背包服务（查询、分页、DO→VO 转换）⚠ .cpp 待实现
│   └── MapService              # 活点地图服务（权限校验 + 查询 + 数据组装）✅ 已实现
│
├── DAO/                        # 数据访问层 - 数据库 CRUD
│   ├── InventoryDAO            # 背包表 JOIN 查询（行解析 + 分页已实现）
│   ├── LocationDAO             # 玩家位置表 CRUD（行解析已实现）
│   ├── UserDAO                 # 用户表
│   ├── CharacterDAO            # 角色表
│   ├── RoomDAO                 # 房间表
│   ├── FriendDAO               # 好友关系表
│   ├── MessageDAO              # 聊天消息表
│   └── NPCDAO                  # NPC 表
│
├── DO/                         # Data Object - 数据库映射对象
│   ├── PlayerInventoryDO       # 玩家背包（inventories + items JOIN 结果）
│   └── PlayerLocationDO        # 玩家位置（player_location 表 1:1 映射）
│
├── VO/                         # View Object - 接口返回对象
│   ├── InventoryListResponseVO # 背包列表响应（含分页信息）
│   └── MapResponseVO           # 活点地图响应（咒语 + 在线人数 + 坐标列表）
│
└── Query/                      # 请求参数对象
    ├── InventoryQuery          # 背包查询参数（characterId, page, pageSize）
    └── MapUseQuery             # 活点地图查询参数（characterId）
```

## 已实现功能

### 活点地图 (Marauder's Map)

使用 `GET /map/use-marauders-map?characterId=xxx` 触发：

1. **权限校验** — 检查玩家背包是否拥有 "Marauder's Map" 道具
2. **位置查询** — 查询当前所有在线玩家的实时位置
3. **数据组装** — DO→VO 转换，附带随机魔法咒语引用

```cpp
// MapService 业务逻辑（已完整实现）
- hasMaraudersMap(characterId)   → 权限校验
- queryOnlinePlayers()           → 查询在线玩家
- DO → VO 转换                   → 返回 MapResponseVO
```

### 玩家背包 (Inventory)

使用 `GET /inventory/list?characterId=xxx&page=x&pageSize=x` 触发：

1. **分页查询** — 支持 page/pageSize 参数
2. **JOIN 查询** — inventories 表 JOIN items 表获取完整道具信息

```cpp
// InventoryService 业务逻辑（.h 已定义，.cpp 待实现）
- getInventory(query)            → 分页查询 + DO→VO 转换
```

## 数据流示例

```
客户端请求
  │
  ▼
MapController.GET /map/use-marauders-map?characterId=42
  │
  ▼
MapService.useMaraudersMap(MapUseQuery)
  ├── InventoryDAO.hasItem(42, "Marauder's Map")  → 是否有道具？
  │     └── 没有 → 返回错误
  │     └── 有   → 继续
  ├── LocationDAO.findAllOnline()                 → 查在线玩家位置
  ├── DO → VO 转换                                 → 组装返回数据
  └── 返回 MapResponseVO                          → 含咒语、人数、坐标列表
```

## 待办事项

- [ ] `InventoryService.cpp` — 背包业务逻辑尚未实现
- [ ] `server/DAO/` 与 `database/include/DAO` 存在代码重复，需统一
- [ ] 将 `server/` 集成到根 CMakeLists.txt 构建
- [ ] 接入真实 MySQL 驱动（当前使用 NoopDriver）
