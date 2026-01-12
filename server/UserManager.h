#pragma once

#include <unordered_map>
#include <string>
#include <vector>
#include <mutex>
#include <memory>
#include "TcpConnection.h" // 假设你的连接类叫这个

class UserManager {
public:
    static UserManager& getInstance() {
        static UserManager instance;
        return instance;
    }

    // 用户登录/活跃时，记录这个连接属于该用户
    void AddConnection(std::string username, TcpConnectionPtr conn) {
        std::lock_guard<std::mutex> lock(m_mutex);
        m_userConnMap[username].push_back(conn);

        // 利用 TcpConnection 的 context 属性存储 username
        conn->setContext(username);
    }

    // 连接断开时，移除记录
    void RemoveConnection(TcpConnectionPtr conn) {
        std::lock_guard<std::mutex> lock(m_mutex);

        // 获取用户名
        if (!conn->hasContext()) return; // 没登录过的连接，直接忽略

        std::string username;
        try {
            username = std::any_cast<std::string>(conn->getContext());
        }
        catch (...) {
            return;
        }

        // 找到该用户的连接列表
        auto it = m_userConnMap.find(username);
        if (it != m_userConnMap.end()) {
            std::vector<TcpConnectionPtr>& connList = it->second;

            connList.erase(std::remove(connList.begin(), connList.end(), conn), connList.end());

            std::cout << "[UserManager] user " << username << " diconnect 1 last: " << connList.size() << std::endl;

            // 如果该用户没有任何连接了，把用户条目也删掉
            if (connList.empty()) {
                m_userConnMap.erase(it);
                std::cout << "[UserManager] user " << username << " is loginout" << std::endl;
            }
        }


    }

    void SendToUser(std::string username, std::string msg) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_userConnMap.find(username);
        if (it != m_userConnMap.end()) {
            std::vector<TcpConnectionPtr>& connList = it->second;

            // 遍历该用户的所有连接
            for (auto connIt = connList.begin(); connIt != connList.end(); ) {
                TcpConnectionPtr conn = *connIt;

                // 发送前检查连接状态
                if (conn && conn->IsConnected()) {
                    conn->SendMessage(msg); // 这里记得加 \n
                    ++connIt;
                }
                else {
                    // 如果发现列表里有死连接，顺手清理掉
                    std::cout << "[UserManager] is disconnected clearing..." << std::endl;
                    connIt = connList.erase(connIt);
                }
            }
        }
        else {
            std::cout << "[UserManager] user " << username << " is disconnected。" << std::endl;
        }

    }
    void Broadcast(const std::string& msg) {
        std::lock_guard<std::mutex> lock(m_mutex);

        // 遍历 map 中所有的用户
        for (auto& pair : m_userConnMap) {
            auto& connList = pair.second;
            for (auto& conn : connList) {
                if (conn->IsConnected()) {
                    conn->SendMessage(msg); // 这是一个 TCP 推送
                }
            }
        }
    }

    // 主动移除某个用户 /logout 请求
    void RemoveUser(const std::string& username) {
        std::lock_guard<std::mutex> lock(m_mutex);

        auto it = m_userConnMap.find(username);
        if (it != m_userConnMap.end()) {
            // 直接从 Map 中删除该用户
            m_userConnMap.erase(it);
            std::cout << "User [" << username << "] removed from manager." << std::endl;
        }
    }

    std::string FindUserByConn(const TcpConnectionPtr& conn) {
        std::lock_guard<std::mutex> lock(m_mutex);

        if (conn->hasContext()) {
            try {
                return std::any_cast<std::string>(conn->getContext());
            }
            catch (...) {
                return "";
            }
        }

        for (auto& pair : m_userConnMap) {
            const std::string& username = pair.first;
            const std::vector<TcpConnectionPtr>& connList = pair.second;

            for (const auto& storedConn : connList) {
                if (storedConn == conn) {
                    return username;
                }
            }
        }

        return ""; // 没找到
    }
private:
    // 一个用户对应多个连接
    std::unordered_map<std::string, std::vector<TcpConnectionPtr>> m_userConnMap;
    std::mutex m_mutex;
};



