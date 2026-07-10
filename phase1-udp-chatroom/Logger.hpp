#ifndef __LOGGER_HPP
#define __LOGGER_HPP

#include <iostream>
#include <string>
#include <ctime>
#include <sstream>
#include <mutex>
#include <unistd.h>
#include <fstream> // 文件流，用于将日志写入文件

namespace NS_LOG_MODULE
{
    // =========================================================================
    // 1. 日志输出策略枚举
    // =========================================================================
    enum class LogStrategy
    {
        Console, // 策略一：输出到标准输出（控制台）
        File     // 策略二：输出到本地日志文件
    };

    // =========================================================================
    // 2. 日志级别枚举
    // =========================================================================
    enum class LogLevel
    {
        DEBUG,   // 调试信息，用于开发阶段定位问题
        INFO,    // 普通信息，记录程序正常运行的关键节点
        WARNING, // 警告信息，存在潜在异常但不影响程序继续运行
        ERROR,   // 错误信息，某个功能模块出现异常，但主流程未终止
        FATAL    // 致命错误，程序无法继续运行，通常需要立即终止
    };

    // =========================================================================
    // 3. 全局日志策略控制器
    // 说明：本头文件可能被多个源文件（.cc/.cpp）包含，若直接定义全局变量
    // 会导致链接期符号重定义。这里利用函数内局部静态变量的特性
    // （C++11起线程安全的静态局部变量初始化），既保证全局唯一，
    // 又避免了多重定义问题。
    // =========================================================================
    inline LogStrategy& GetGlobalStrategy() {
        static LogStrategy strategy = LogStrategy::Console; // 默认策略：输出到控制台
        return strategy;
    }

    // 将日志级别枚举转换为对应的可读字符串
    inline std::string LogLevel2Message(LogLevel level)
    {
        switch (level)
        {
        case LogLevel::DEBUG:   return "DEBUG";
        case LogLevel::INFO:    return "INFO";
        case LogLevel::WARNING: return "WARNING";
        case LogLevel::ERROR:   return "ERROR";
        case LogLevel::FATAL:   return "FATAL";
        default:                return "UNKNOWN";
        }
    }

    // 获取当前系统时间并格式化为字符串（例如：2026-07-07 15:40:08）
    inline std::string GetCurrentTime()
    {
        time_t now = time(nullptr);
        struct tm *lt = localtime(&now);
        char time_str[64];
        strftime(time_str, sizeof(time_str), "%Y-%m-%d %H:%M:%S", lt);
        return time_str;
    }

    // =========================================================================
    // 4. 日志器核心类
    // 设计原理：利用临时对象的生命周期实现类似 std::cout 的链式调用。
    // 调用 LOG(INFO) << "xxx" 时会构造一个 Logger 临时对象，
    // 后续通过重载的 operator<< 将内容依次写入内部缓冲区 _ss；
    // 当该语句执行结束、临时对象析构时，统一在析构函数中加锁并输出，
    // 从而保证多线程环境下单条日志的完整性（不会被其他线程的日志内容打断）。
    // =========================================================================
    class Logger
    {
    public:
        // 构造函数：对象构造时即拼装好日志头部信息（级别、时间戳、进程号、源码位置）
        Logger(LogLevel level, const std::string &file, int line)
            : _level(level)
        {
            _ss << "[" << LogLevel2Message(_level) << "]"
                << "[" << GetCurrentTime() << "]"
                << "[" << getpid() << "]"
                << "[" << file << ":" << line << "] ";
        }

        // 仿照 std::cout 的用法，通过重载运算符支持任意可流式输出类型的拼接
        template <typename T>
        Logger &operator<<(const T &val)
        {
            _ss << val;
            return *this;
        }

        // 析构函数：临时对象生命周期结束时，根据当前策略将日志内容输出到目标位置
        ~Logger()
        {
            // 静态局部互斥锁：避免多线程并发写日志时出现内容交叉、错乱的问题
            static std::mutex mtx;
            std::lock_guard<std::mutex> lock(mtx);

            // 根据全局策略决定日志的输出目的地
            if (GetGlobalStrategy() == LogStrategy::Console)
            {
                // 策略一：输出到标准输出
                std::cout << _ss.str() << std::endl;
            }
            else if (GetGlobalStrategy() == LogStrategy::File)
            {
                // 策略二：以追加模式写入本地日志文件
                std::ofstream log_file("server.log", std::ios::app);
                if (log_file.is_open())
                {
                    log_file << _ss.str() << std::endl;
                }
                else
                {
                    // 文件打开失败时，降级输出到标准错误流，避免日志丢失
                    std::cerr << "[LOG_ERR] Failed to open server.log! fallback: " << _ss.str() << std::endl;
                }
            }
        }

    private:
        LogLevel _level;
        std::stringstream _ss; // 缓冲区，暂存单条日志的拼装内容
    };
}

// =========================================================================
// 5. 日志接口宏定义
// =========================================================================

// 通过预处理宏 __FILE__ 和 __LINE__ 自动获取当前日志触发点的文件名和行号
#define LOG(level) NS_LOG_MODULE::Logger(NS_LOG_MODULE::LogLevel::level, __FILE__, __LINE__)

// 将全局日志策略切换为控制台输出
#define ENABLE_CONSOLE_LOG_STRATEGY() do { NS_LOG_MODULE::GetGlobalStrategy() = NS_LOG_MODULE::LogStrategy::Console; } while(0)

// 将全局日志策略切换为文件输出（写入当前目录下的 server.log）
#define ENABLE_FILE_LOG_STRATEGY() do { NS_LOG_MODULE::GetGlobalStrategy() = NS_LOG_MODULE::LogStrategy::File; } while(0)

#endif
