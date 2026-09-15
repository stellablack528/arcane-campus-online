# HogwartsServer 网络模块测试报告 & 保姆级测试指南

> 本文件记录网络模块（`src/network/`）的 bug 修复情况与连接测试结果，
> 并给出完整的 WSL 手动测试步骤。任何人 clone 仓库后都能照此复现。

---

## 一、网络模块概述

网络模块是霍格沃兹在线校园游戏的服务器后端 TCP 层，采用 **Linux POSIX socket** 实现
（不使用 Winsock，因为生产环境部署在 Linux 服务器）。

```
src/network/
├── Socket.hpp / Socket.cpp          # Linux socket 封装（create/bind/listen/accept/close）
├── TcpConnection.hpp / .cpp         # 单个客户端连接（send/receive + 消息回调）
├── TcpServer.hpp / .cpp             # TCP 服务器（Start/acceptClient/connectionCount）
└── ConnectionManager.hpp / .cpp     # 连接管理器（add/remove/find/size）
```

测试入口：`src/server/server_main.cpp`，流程为：
```
启动 Server (端口 8888)
    ↓
等待客户端连接
    ↓
acceptClient() → ConnectionManager 添加连接
    ↓
打印 connectionCount()
```

---

## 二、本次修复的 Bug

### Bug 1：`Socket.cpp` 析构函数未判断 fd 有效性

**修复前：**
```cpp
Socket::~Socket()
{
    ::close(fd_);   // 默认构造时 fd_==-1，::close(-1) 触发 EBADF
}
```

**修复后：**
```cpp
Socket::~Socket()
{
    if (fd_ >= 0) {
        ::close(fd_);
        fd_ = -1;
    }
}
```

**影响**：默认构造的 Socket 对象析构时不再产生无效系统调用。

---

### Bug 2：`TcpServer.cpp` 析构函数未关闭监听 Socket

**修复前：**
```cpp
TcpServer::~TcpServer()
{
    // 空 — listenSocket_ 的 fd 泄漏
}
```

**修复后：**
```cpp
TcpServer::~TcpServer()
{
    listenSocket_.close();   // 显式释放监听 fd
}
```

**影响**：虽然进程退出时 OS 会回收 fd，但显式关闭是规范做法，避免未来
长生命周期场景下的 fd 泄漏。

---

### Bug 3：`TcpConnection.cpp` 缺少 `<sys/types.h>` 头文件

**修复前**：代码使用 `ssize_t`（POSIX 类型）但未显式包含 `<sys/types.h>`，
依赖 `<sys/socket.h>` 间接引入，严格说不保证可移植。

**修复后**：显式 `#include <sys/types.h>`。

---

### Bug 4：`TcpConnection.cpp` send/recv 的 errno 判断不严谨

**修复前：**
```cpp
ssize_t result = ::send(...);
if (result > 0) { ... continue; }
if (result == 0) { return false; }
if (errno == EINTR) { continue; }   // 隐含 result<0 但未显式写
return false;
```

**修复后：**
```cpp
if (result < 0 && errno == EINTR) { continue; }
// recv 同理：
if (result < 0 && (errno == EAGAIN || errno == EWOULDBLOCK)) { return; }
```

**影响**：errno 只在返回值 < 0 时有定义。显式 `result < 0` 判断使逻辑更清晰，
避免误读 errno。

---

### Bug 5：`Socket.cpp` accept() 函数缩进混乱

**修复前**：缩进导致两个 return 看似一个在 if 外一个在 if 内，容易误读。

**修复后**：整理为标准缩进，逻辑清晰（错误时打印日志后返回负数 fd）。

---

### 改进：`server_main.cpp` 加 `std::ios::unitbuf`

```cpp
std::cout.setf(std::ios::unitbuf);   // 每次输出立即 flush
```

**原因**：调试服务器在后台/管道场景下 std::cout 默认块缓冲，导致连接日志
不及时显示。加 unitbuf 后每次 cout 输出立即 flush，方便调试观察。

---

## 三、测试环境

| 项目 | 版本 |
|------|------|
| 操作系统 | Windows 11 + WSL2 Ubuntu |
| WSL 内核 | 6.18.33.2-microsoft-standard-WSL2 |
| 编译器 | g++ (Ubuntu 15.2.0-16ubuntu1) 15.2.0 |
| C++ 标准 | C++17 |
| 编译选项 | -Wall -Wextra -Wpedantic |
| 客户端工具 | nc (netcat-openbsd) |
| 端口 | 8888 |

---

## 四、测试结果

### 编译测试
```
$ g++ -std=c++17 -Wall -Wextra -Wpedantic server_main.cpp \
    ../network/Socket.cpp ../network/TcpConnection.cpp \
    ../network/TcpServer.cpp ../network/ConnectionManager.cpp \
    -o HogwartsServer
```
**结果**：编译成功，无 warning 无 error，生成 HogwartsServer 可执行文件（108KB）。

### 连接测试

启动 server 后，用 nc 模拟两个客户端依次连接：

```
Server started on port 8888.
=== nc 连接 1 ===
Client connected, fd = 4, current connections = 1
=== nc 连接 2 ===
Client connected, fd = 5, current connections = 2
```

| 客户端 | 返回 fd | connectionCount |
|--------|---------|-----------------|
| 客户端 1 | 4 | 1 |
| 客户端 2 | 5 | 2 |

**结论**：✅ 连接流程完全符合预期——acceptClient() 正确返回 fd，
ConnectionManager 正确累加连接数。

---

## 五、保姆级测试教程（你照着做就行）

### 前提条件
- 已装 WSL2 + Ubuntu（你电脑已经有了）
- WSL Ubuntu 里有 g++（一般自带，没有就 `sudo apt install g++`）
- WSL Ubuntu 里有 nc（没有就 `sudo apt install netcat-openbsd`）

### 第 1 步：打开 WSL Ubuntu 终端

在 Windows 搜索栏搜 "Ubuntu" 打开，或者在 PowerShell/CMD 里输入：
```
wsl -d Ubuntu
```

### 第 2 步：进入 server 目录

```bash
cd /mnt/d/HogwartsOnline/HogwartsOnline/src/server
```

> `/mnt/d/` 就是 Windows 的 D 盘在 WSL 里的挂载路径。

### 第 3 步：编译

```bash
g++ -std=c++17 server_main.cpp ../network/Socket.cpp ../network/TcpConnection.cpp ../network/TcpServer.cpp ../network/ConnectionManager.cpp -o HogwartsServer
```

没报错就是成功，会生成一个 `HogwartsServer` 文件。

### 第 4 步：启动服务器

```bash
./HogwartsServer
```

看到这行就说明起来了：
```
Server started on port 8888.
```

> 这个终端现在被 server 占着（它在等客户端连进来），**不要关它**。

### 第 5 步：开第二个终端，当客户端 1

新开一个 WSL Ubuntu 终端（再搜一次 Ubuntu 打开，或者 `wsl -d Ubuntu`），
输入：

```bash
nc 127.0.0.1 8888
```

回到第 4 步那个终端看，应该出现：
```
Client connected, fd = 4, current connections = 1
```

> `nc` 连上后会等你输入文字，你随便打几个字回车，server 端不会显示
> （当前版本只计数不回显），这不影响测试。这个终端也先开着。

### 第 6 步：开第三个终端，当客户端 2

再开一个 WSL 终端，输入一样的：
```bash
nc 127.0.0.1 8888
```

回 server 终端看，应该出现：
```
Client connected, fd = 5, current connections = 2
```

到这一步，两个客户端都连上了，`connectionCount` 从 1 变 2，测试成功！

### 第 7 步：收尾

- **客户端终端**（nc）：按 `Ctrl+C` 退出
- **服务器终端**：按 `Ctrl+C` 停止 server

### 故障排查

| 问题 | 解决 |
|------|------|
| `bind failed: Address already in use` | 上次的 server 没关干净，跑 `pkill -x HogwartsServer` 再启动 |
| `g++: command not found` | `sudo apt install g++` |
| `nc: command not found` | `sudo apt install netcat-openbsd` |
| 连上了但 server 没显示 | 确认 server_main.cpp 有 `std::cout.setf(std::ios::unitbuf);` |

---

## 六、用 CMake 编译（可选，更规范）

WSL Ubuntu 里如果装了 cmake（`sudo apt install cmake`），可以用独立
CMakeLists 编译：

```bash
cd /mnt/d/HogwartsOnline/HogwartsOnline/src/server
mkdir build && cd build
cmake .. && make
./HogwartsServer
```

这个 CMakeLists 与根 CMakeLists.txt **完全解耦**——根 CMake 仍然管
Qt6 + MySQL + 主程序（HogwartsOnline.exe），这里只负责网络模块的
Linux 编译测试，互不影响。
