# 小型校园联机：Qt TCP 实现与学习路线

这一版只同步场景聊天、进出场景和在线猫头鹰。角色存档、剧情判定、背包和学期进度仍由游戏自己的逻辑负责；换网络库不会解决这些游戏规则问题。

## 找到好库时，先决定要借什么

| 我需要什么 | 怎么做 | 这次的选择 |
| --- | --- | --- |
| 仅参考一个设计 | 读示例、确认许可证和文档，用项目现有工具实现相同职责 | 借鉴 Asio 的连接、聊天室、按连接有序发送；未复制或引入 Asio 源码 |
| 需要库提供的能力 | 用 CMake 引入固定版本，先写最小样例；在自己的适配层里包住第三方 API | Qt Network 已能承担 I/O，不加第二个网络库 |
| 库确实缺功能，且必须改它本身 | 优先提交上游修复；若等不了，再 Fork **整个仓库**，把自家改动控制在少数提交 | 此处没有 Fork 的必要 |

Fork 不是勾选几个目录：它建立整个仓库的派生副本。若只想搬少量文件，先核对许可证及归属要求；技术上还要确认这些文件用到哪些头文件、运行库、事件循环和平台 API。原型先验证 Windows/MinGW、Qt 事件循环、消息格式与错误行为，再决定接入。不要同时在同一个连接上让 Asio 和 Qt 读写。

这次参考的是 [Qt Fortune Client](https://github.com/qt/qtbase/blob/dev/examples/network/fortuneclient/client.cpp) 的完整消息读取思路，以及 [Asio chat_server](https://github.com/chriskohlhoff/asio/blob/master/src/examples/cpp11/chat/chat_server.cpp) 的聊天室与连接职责。两个示例的**协议都不直接兼容**本项目：Qt 示例用 `QDataStream`，Asio 示例用长度头，我们保留现有的 UTF-8 JSON + 换行分帧。Qt 自带按序写缓冲，所以参考 Asio 的有序发送思路，但不复制它的队列，改用 `bytesToWrite()` 限住 Qt 的待发数据。

## 按这个顺序读代码

1. `CampusWireProtocol.*`：两端共用的分帧与限制。`readyRead` 表示有新字节，不表示有一条完整消息。可能只到半条，也可能一次到多条。
2. `CampusWireServer.*`：握手通过才登记在线角色。普通消息与进出提示按 `(era, scene)` 广播，猫头鹰按 `(era, nickname)` 找人。小房间用遍历即可，不需要先上线程池或房间索引。
3. `CampusWireClient.*`：TCP 连上之后还要等 `welcome`。掉线后客户端以 1、2、4、8 秒间隔（最高 8 秒）重试；它记住的是游戏当前场景，而不是上次联网时的场景。
4. `../../tests/network/CampusWireTest.cpp`：用真实的回环 TCP 连接检查同场景、跨场景、跨时代、消息边界和重连。

`src/main.cpp` 只是接线：世界时代从剧情控制器传给客户端，开放探索后才提交角色与地点；本机房主逻辑仍然保留。目录里的 `Socket/TcpConnection/TcpServer` 是另一路实现，不是 Qt 试玩版的传输层，这次未改动。

## 下次网络博客

**TCP 负责把字节送到，协议负责拼出一句话，房间规则决定谁能听见。**

用一条「大礼堂里的发言」串起三层：Qt TCP 收到字节，`readFrame()` 按换行还原 JSON，服务端再按 `(era, scene)` 决定发给谁。

## 协议与边界

- `hello` 示例：`{"type":"hello","version":2,"name":"Alice","era":"parents","location":"great_hall"}`。每个方向每条 JSON 对象末尾加一个 LF。服务端回 `welcome`，客户端也检查其版本和时代，之后才能发送 `move`、`say`、`owl`。
- 版本和时代必须显式匹配。旧版不能按时代分房，客户端与独立服务端请一起升级。房间只使用稳定 ID，不使用当前 UI 语言的显示名称。
- 单帧最多 4096 字节（不含 LF），每条聊天/信件正文最多 500 个 UTF-16 代码单元。服务端限制待握手时间与连接数（32），两端均限制读取缓冲和待发送队列。32 是保护上限，不是压测结果。
- 联机仍仅适合你信任的本机或局域网测试。时代、昵称是客户端声明，不是登录凭证；连接没有 TLS。重连是重新加入，不是账号续期，不补发离线聊天或猫头鹰；“已发送”表示服务器已将信件加入接收连接的发送缓冲，不是已读。

## 怎么验证

从仓库根目录执行（目录可换成自己 Qt Creator 的构建目录）：

```powershell
cmake --build build/qtcreator-debug --target ArcaneCampusWireTest ArcaneCampusLiveServer HogwartsOnline
ctest --test-dir build/qtcreator-debug -R '^ArcaneCampusWireTest$' --output-on-failure
```

同机也可开两个客户端；不同电脑请在可信局域网内使用同版本客户端和独立服务端。服务端启动前将 `ARCANE_SERVER_BIND` 设为房主的局域网 IP；客户端启动前将 `ARCANE_SERVER_HOST` 设为同一 IP，双方使用同一个 `ARCANE_SERVER_PORT`（默认 8888）。不要把端口转发到公网。开三个角色验收时，两个选同一时代，一个选不同的时代，完成第一堂魔咒课后去大礼堂：同年代能听见；换去图书馆后还能收到同年代的猫头鹰，但不会收到大礼堂发言；不同时代互相隐形。
