# Arcane Campus Online 🏰

> A multiplayer online text RPG social platform built with C++ and Linux network programming.
>
> 基于 C++ 与 Linux 网络编程构建的多人在线文字 RPG 社交平台。

---

## 项目简介 / About
### 中文

**Arcane Campus Online** 是一个以魔法校园为背景的多人在线文字 RPG 社交平台。

项目从一个轻量级 Socket 聊天室开始，逐步演进为支持多场景、多角色、实时互动的虚拟校园世界。

玩家以角色身份进入世界，在礼堂、图书馆、天文塔、公共休息室等场景中进行聊天、社交、探索与互动。

除了构建一个完整的文字世界之外，本项目也希望成为一个持续迭代的 Linux 网络编程实践平台，用于探索现代 C++ 后端开发中的高并发网络模型、数据库设计、缓存架构以及客户端开发技术。

> 本项目为非商业同人作品。
> Harry Potter © J.K. Rowling / Warner Bros. 版权所有。


### English

**Arcane Campus Online** is a multiplayer online text RPG social platform inspired by the magical world of Harry Potter.

Starting from a lightweight socket chatroom, the project gradually evolves into a multi-scene, multi-character, real-time interactive virtual campus.

Players enter the world as characters and interact with others in locations such as the Great Hall, Library, Astronomy Tower, Dormitories, and more. The platform aims to provide an immersive text-based social experience while serving as a practical playground for modern C++ backend architecture and Linux network programming.

The project focuses on building a scalable server architecture and exploring technologies commonly used in large-scale online systems, including asynchronous networking, concurrency models, database integration, caching systems, and desktop client development.

>  Fan Project — Non-commercial use only.
> Harry Potter © J.K. Rowling / Warner Bros. All rights reserved.

---


---

## 技术栈 / Tech Stack

| 层次 / Layer              | 技术 / Technology                       |
| ----------------------- | ------------------------------------- |
| 开发语言 / Language         | C++20                                 |
| 操作系统 / Operating System | Ubuntu Linux                          |
| 网络模型 / Network Model    | TCP Socket · Epoll · Reactor Pattern  |
| 并发模型 / Concurrency      | Thread Pool · Producer-Consumer Model |
| 数据库 / Database          | MySQL                                 |
| 缓存系统 / Cache            | Redis                                 |
| 客户端 / Client            | Qt6 Widgets                           |
| 构建工具 / Build System     | CMake                                 |

---

## 系统架构 / Architecture

```text
Qt Desktop Client
        │
 TCP Long Connection
        │
   Gateway Server
        │
┌──────────────────────┐
│      Room System     │
│      Chat System     │
│   Character System   │
│      NPC System      │
│      Quest System    │
└──────────────────────┘
        │
   MySQL + Redis
```

---

## 当前进度 / Progress

* [x] UDP Chatroom Demo
* [x] Thread Pool
* [x] Producer-Consumer Architecture
* [x] Modular Project Structure
* [ ] TCP Persistent Connection
* [ ] Room System
* [ ] Character System
* [ ] Epoll-based High Concurrency Refactor
* [ ] MySQL Integration
* [ ] Redis Integration
* [ ] Qt Desktop Client
* [ ] AI NPC System

---

## Planned Features

* Real-time scene chat
* Multi-room interaction
* Character and profile system
* Friend system
* Inventory system
* Quest system
* NPC interaction
* Achievement system
* AI-driven NPC dialogue
* Persistent world data storage

---

## 项目目标 / Goals

* Practice Linux network programming and high-concurrency server design.
* Explore modern C++ backend engineering practices.
* Build a long-term portfolio project for graduate school applications and future career development.
* Accumulate experience in system architecture, database design, and desktop application development.

---

## 作者 / Author

Third-year CS student · Linux Network Programming Enthusiast

Currently focusing on:

* Modern C++
* Linux Network Programming
* High-Concurrency Server Architecture
* Distributed Systems
* Database Engineering

Building this as a long-term portfolio project for graduate school applications and backend engineering practice. 

---

## License

MIT License — See [LICENSE](./LICENSE) for details.

> Fan Project · Non-commercial use only.
>
> Harry Potter © J.K. Rowling / Warner Bros.
A multiplayer online text RPG social platform built with C++ and Linux network programming
