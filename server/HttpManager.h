#pragma once

#include "Buffer.h"
#include <string>
#include "Callbacks.h"

#include "json.hpp"

using json = nlohmann::json;


/*
POST /login HTTP/1.1
Host: 192.168.84.129:2552
Content-Type: application/json
Content-Length: 91
Connection: Keep-Alive
Accept-Encoding: gzip, deflate
Accept-Language: zh-CN,en,*
User-Agent: Mozilla/5.0
                    // 这里就是连续两个\r\n导致的
{"sign":"9ffc4cdae921c82421057efd8090155f","timestamp":1764746979,"type":0,"username":"yejisu"}
*/

class HttpRequest {
public:
    bool Parse(Buffer* buf) {
        const char* crlf_crlf = GetCrlf(buf);
        if (crlf_crlf == nullptr) return false;
        std::string_view header = ParseHttpHeader(buf, crlf_crlf);
        m_action = GetUrlAction(header); // 获取 /login
        std::string json_request = GetJsonRequest(buf, crlf_crlf, header);
        if (json_request == "") return false;
        m_jsonData = json::parse(json_request);
        return true;
    }
private:
    // 获取json内容
    std::string GetJsonRequest(Buffer* buf, const char* crlf_crlf, std::string_view& header) {
        // 解析内容长度
        size_t content_length = GetContentLength(header);

        // 检查我们是否收到了完整的 JSON 正文
        const char* body_start = crlf_crlf + 4;
        size_t total_len = (body_start - buf->peek()) + content_length;
        if (buf->ReadableBytes() < total_len) return "";

        // 拿到完整的HTTP请求
        std::string json_request(body_start, content_length);
        buf->rm_ReadIndex(total_len);
        //std::cout << "Thread received JSON Body: " << json_request << std::endl;
        return json_request;
    }
    // 获取请求类型
    std::string GetUrlAction(std::string_view& header) {
        size_t first_line_end = header.find("\r\n"); // 指向第一行末尾
        if (first_line_end != std::string_view::npos) {
            std::string_view head = header.substr(0, first_line_end); // POST /login HTTP/1.1
            
            size_t first_space = head.find(' ');
            if (first_space == std::string_view::npos) return ""; // 没找到空格
            size_t second_space = head.find(' ', first_space + 1);
            if (second_space == std::string_view::npos) return ""; // 没找到第二个空格

            // 提取中间的部分(/login)
            std::string url_action = std::string(head.substr(first_space + 1, second_space - first_space - 1));
            return url_action;
        }
        return "";
    }
    // 获取json数据长度
    size_t GetContentLength(std::string_view header) {
        // 转为小写，解决大小写不敏感问题
        std::string lower_headers;
        lower_headers.resize(header.size());
        std::transform(header.begin(), header.end(), lower_headers.begin(), ::tolower);
        // 查找 "content-length: "
        size_t len_pos = lower_headers.find("content-length: ");
        if (len_pos == std::string_view::npos) {
            return 0;
        }
        len_pos += 16; // 跳过 "content-length: "
        size_t len_end = lower_headers.find("\r\n", len_pos);
        try {
            return std::stoul(std::string(header.substr(len_pos, len_end - len_pos)));
        }
        catch (...) {
            return 0;
        }
    }
    // 拿到http头部末尾指针
    const char* GetCrlf(Buffer* buf) {
        const char* crlf_crlf = buf->FindCRLFCRLF(); 
        if (crlf_crlf == nullptr) { // HTTP 头部还不完整，退出 while 循环，继续等待         
            return nullptr;
        }
        return crlf_crlf;
    }
    std::string_view ParseHttpHeader(Buffer* buf, const char* crlf_crlf) {
        size_t header_len = crlf_crlf - buf->peek();
        std::string_view header(buf->peek(), header_len);
        return header;
    }
public:
    std::string m_action;
    json m_jsonData;
};

class HttpResponse {
public:
   
    static void SendHttpResponse(const TcpConnectionPtr& conn, json& json_response, const std::string& http_status = "200 OK")
    {
        std::string json_str = json_response.dump(-1);
        std::string http_response;
        http_response.reserve(json_str.size() + 128); // 预分配内存

        http_response += "HTTP/1.1 " + http_status + "\r\n";
        http_response += "Content-Type: application/json; charset=utf-8\r\n";
        http_response += "Content-Length: " + std::to_string(json_str.size()) + "\r\n";
        http_response += "Connection: keep-alive\r\n"; // Keep-Alive
        http_response += "\r\n"; // 空行
        http_response += json_str;

        conn->SendMessage(http_response);

    }
    static json SetError(const std::string& msg) {
        json j_error;
        j_error["code"] = 400;
        j_error["message"] = msg;
        j_error["status"] = "fail";
        return j_error;
    }
};