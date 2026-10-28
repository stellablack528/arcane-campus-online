# Qt Creator 本地试玩

## 打开正确工程

用 Qt Creator 打开本仓库根目录的 `CMakeLists.txt`，选择 **Desktop Qt 6.10.1 MinGW 64-bit** Kit。构建并运行的目标必须是 `HogwartsOnline`。当前主界面以中央叙事流为主体，两侧功能是抽屉；如果运行后仍看到旧版三栏校园主页，请检查 Qt Creator“项目 → 运行”中的可执行文件路径，再执行“构建 → 重新构建项目”。

## 连接数据库

首次在全新的 MySQL 数据库试玩时，在仓库根目录运行 `database/scripts/import_mysql.ps1`。已有角色存档时不要重导整个种子数据。

把 `database/config/database.ini.example` 复制为 `database.ini`，确认 `client_library_path` 指向实际 DLL，例如 `D:/mysql-8.0.25-winx64/lib/libmysql.dll`。在 Qt Creator“项目 → 运行 → 环境”中设置：

```text
ARCANE_DB_CONFIG=D:/HogwartsOnline/HogwartsOnline/database.ini
ARCANE_DB_PASSWORD=你的本地 MySQL 密码
```

如果已有数据库是旧版本，但缺少庭院房间，只执行以下幂等 SQL；无需重新导入示例角色：

```sql
USE arcane_campus_online;
INSERT INTO rooms (room_name, room_type, description, max_players, is_restricted)
VALUES ('Courtyard', 'public', 'A quiet courtyard beside the fountain.', 120, FALSE)
ON DUPLICATE KEY UPDATE room_name = VALUES(room_name);
```

数据库连接失败时，正常启动的客户端会停留在登录页并显示原因，不会把演示校园误当作新生剧情。`ARCANE_DEMO_MODE=1` 仅供需要旧演示模式时使用；演示模式不支持新生注册或剧情存档。

## 从头游玩

点击“创建新生角色”，填写用户名、角色信息和魔杖线索。登记后在中央叙事框进入对角巷，跟随选项走进奥利凡德魔杖店，再完成购物、入学旅程、分院和第一堂魔咒课。学会漂浮咒后可去礼堂、图书馆、庭院调查自己翻页的书。无需 DeepSeek API Key，也无需启动联机服务端。

已有角色使用注册时的**用户名**和学院点击“继续已有角色”，不要把角色昵称当用户名。想从头再体验不同选择，请创建一个新用户名。

联机试玩时，另外启动 `ArcaneCampusLiveServer`，再打开两个客户端；它们默认连接本机 `127.0.0.1:8888`。
