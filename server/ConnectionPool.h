#pragma once

#include <queue>
#include <mutex>
#include <condition_variable>
#include <memory>
#include <atomic>
#include "MysqlConn.h"

class ConnectionPool
{
public:
    // 单例模式,获取全局唯一的连接池
    static ConnectionPool* getConnectionPool() {
        static ConnectionPool pool;
        return &pool;
    }

    // 从池中获取一个连接 
    std::shared_ptr<MysqlConn> getConnection() {
        std::unique_lock<std::mutex> lock(_queueMutex);
        while (_connectionQueue.empty()) {
            _cv.wait(lock); // 没连接就等着
        }

        MysqlConn* conn = _connectionQueue.front();
        _connectionQueue.pop();

        // 当shared_ptr销毁时，不delete，而是把连接放回队列
        return std::shared_ptr<MysqlConn>(conn, [this](MysqlConn* pConn) {
            std::unique_lock<std::mutex> lock(_queueMutex);
            pConn->refreshAliveTime();
            _connectionQueue.push(pConn);
            _cv.notify_one();
            });
    }

private:
    // 构造函数：初始化池子
    ConnectionPool() {
        // 配置信息
        _ip = "127.0.0.1"; // 在linux里运行
        _port = 3306;
        _user = "admin";   // 用户名
        _passwd = "196950"; // 你的密码
        _dbName = "Sys_db";
        _initSize = 5;      // 初始连接数

        for (int i = 0; i < _initSize; ++i) {
            MysqlConn* conn = new MysqlConn();
            if (!conn->connect(_user, _passwd, _dbName, _ip, _port)) {
                std::cerr << "Connect Database Failed!" << std::endl;
                continue; // 连接失败处理
            }
            conn->refreshAliveTime();
            _connectionQueue.push(conn);
        }
    }

    std::string _ip, _user, _passwd, _dbName;
    unsigned short _port;
    int _initSize;

    std::queue<MysqlConn*> _connectionQueue;
    std::mutex _queueMutex;
    std::condition_variable _cv;
};

