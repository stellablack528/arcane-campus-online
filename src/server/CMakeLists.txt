# src/server/CMakeLists.txt
#
# 独立的网络模块测试可执行文件，**完全不依赖 Qt6 / MySQL**。
# 用法（WSL Ubuntu 里）：
#   cd /mnt/d/HogwartsOnline/HogwartsOnline/src/server
#   mkdir build && cd build
#   cmake .. && make
#   ./HogwartsServer
#
# 或不用 cmake，直接 g++ 一行：
#   g++ -std=c++17 server_main.cpp ../network/*.cpp -o HogwartsServer && ./HogwartsServer
#
# 这个 CMakeLists 与根 CMakeLists.txt 完全解耦——根 CMake 仍然管
# Qt6 + MySQL + 主程序（HogwartsOnline.exe），这里只负责 network 模块
# 的 Linux 编译测试。VS2022 想 WSL 调试时，单独 Open Folder 指向
# src/server 即可。

cmake_minimum_required(VERSION 3.16)
project(HogwartsServer LANGUAGES CXX)

set(CMAKE_CXX_STANDARD 17)
set(CMAKE_CXX_STANDARD_REQUIRED ON)
set(CMAKE_CXX_EXTENSIONS OFF)

# network 模块源码在同级 ../network 目录。
set(NETWORK_DIR ${CMAKE_CURRENT_SOURCE_DIR}/../network)

add_executable(HogwartsServer
    server_main.cpp
    ${NETWORK_DIR}/Socket.cpp
    ${NETWORK_DIR}/TcpConnection.cpp
    ${NETWORK_DIR}/TcpServer.cpp
    ${NETWORK_DIR}/ConnectionManager.cpp
)

# 让 server_main.cpp 既能写 #include "../network/TcpServer.hpp"
# 也能写 #include "TcpServer.hpp"（更规范）。
target_include_directories(HogwartsServer PRIVATE ${NETWORK_DIR})

# 开发期警告：未使用变量、未初始化、shadow 等。
target_compile_options(HogwartsServer PRIVATE -Wall -Wextra -Wpedantic)
