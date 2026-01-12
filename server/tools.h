#pragma once

#include <iostream>
#include <vector>

static const std::string base64_chars = "ABCDEFGHIJKLMNOPQRSTUVWXYZabcdefghijklmnopqrstuvwxyz0123456789+/";
static inline bool is_base64(unsigned char c) { return (isalnum(c) || (c == '+') || (c == '/')); }

std::vector<unsigned char> base64_decode(std::string const& encoded_string) {
    int in_len = encoded_string.size();
    int i = 0, j = 0, in_ = 0;
    unsigned char char_array_4[4], char_array_3[3];
    std::vector<unsigned char> ret;

    while (in_len-- && (encoded_string[in_] != '=') && is_base64(encoded_string[in_])) {
        char_array_4[i++] = encoded_string[in_]; in_++;
        if (i == 4) {
            for (i = 0; i < 4; i++) char_array_4[i] = base64_chars.find(char_array_4[i]);
            char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
            char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
            char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
            for (i = 0; (i < 3); i++) ret.push_back(char_array_3[i]);
            i = 0;
        }
    }
    if (i) {
        for (j = i; j < 4; j++) char_array_4[j] = 0;
        for (j = 0; j < 4; j++) char_array_4[j] = base64_chars.find(char_array_4[j]);
        char_array_3[0] = (char_array_4[0] << 2) + ((char_array_4[1] & 0x30) >> 4);
        char_array_3[1] = ((char_array_4[1] & 0xf) << 4) + ((char_array_4[2] & 0x3c) >> 2);
        char_array_3[2] = ((char_array_4[2] & 0x3) << 6) + char_array_4[3];
        for (j = 0; (j < i - 1); j++) ret.push_back(char_array_3[j]);
    }
    return ret;
}

std::string base64_encode(const unsigned char* data, size_t length) {

    std::string ret;
    ret.reserve(((length + 2) / 3) * 4); // 预分配内存

    size_t i = 0;
    while (i < length) {
        // 显式获取三个字节，把 i 的自增逻辑拿出来，避免三元运算符短路
        unsigned int octet_a = (i < length) ? data[i++] : 0;
        unsigned int octet_b = (i < length) ? data[i++] : 0;
        unsigned int octet_c = (i < length) ? data[i++] : 0;

        unsigned int triple = (octet_a << 0x10) + (octet_b << 0x08) + octet_c;

        // 填充输出
        ret += base64_chars[(triple >> 3 * 6) & 0x3F];
        ret += base64_chars[(triple >> 2 * 6) & 0x3F];

        ret += (i > length + 1) ? '=' : base64_chars[(triple >> 1 * 6) & 0x3F];
        ret += (i > length) ? '=' : base64_chars[(triple >> 0 * 6) & 0x3F];
    }
    return ret;
}

// 重载 接受 std::vector<char> 或 std::string
std::string base64_encode(const std::vector<char>& data) {
    return base64_encode(reinterpret_cast<const unsigned char*>(data.data()), data.size());
}

std::string base64_encode(const std::string& data) {
    return base64_encode(reinterpret_cast<const unsigned char*>(data.data()), data.size());
}