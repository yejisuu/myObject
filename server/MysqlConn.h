#pragma once

#include <mysql/mysql.h>
#include <string>
#include <chrono>
#include <iostream>

class MysqlConn
{
public:
    MysqlConn() {
        conn_ = mysql_init(nullptr);
        if (conn_) {
            mysql_set_character_set(conn_, "utf8mb4");
        }
    }

    // 释放资源
    ~MysqlConn() {
        if (conn_ != nullptr) {
            mysql_close(conn_);
        }
    }

public:
    bool connect(std::string user, std::string passwd, std::string dbName, std::string ip, unsigned short port = 3306) {
        MYSQL* p = mysql_real_connect(conn_, ip.c_str(), user.c_str(), passwd.c_str(), dbName.c_str(), port, nullptr, 0);
        return p != nullptr;
    }
    bool update(std::string sql) {
        if (mysql_query(conn_, sql.c_str())) {
            std::cerr << "Update Failed: " << sql << std::endl;
            return false;
        }
        return true;
    }
    // 查询
    MYSQL_RES* query(std::string sql) {
        if (mysql_query(conn_, sql.c_str())) {
            std::cerr << "Query Failed: " << sql << std::endl;
            return nullptr;
        }
        return mysql_store_result(conn_);
    }

    // 刷新空闲时间
    void refreshAliveTime() { _alivetime = std::chrono::steady_clock::now(); }

    // 获取空闲时长 
    long long getAliveTime() {
        std::chrono::nanoseconds res = std::chrono::steady_clock::now() - _alivetime;
        return std::chrono::duration_cast<std::chrono::milliseconds>(res).count();
    }
private:

	MYSQL* conn_;
	std::chrono::steady_clock::time_point _alivetime;
};

