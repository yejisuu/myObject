#pragma once

#include <string>
#include <vector>
#include <iostream>
#include <algorithm>
class Buffer
{
public:
	Buffer(size_t size = m_initSize) {
		m_buffer.resize(size);
		m_readIndex = 0;
		m_writeIndex = 0;
	}
	~Buffer() {}
public:
	size_t ReadableBytes() const { return m_writeIndex - m_readIndex; } // 可读字节数  右 - 左
	size_t WriteableBytes() const { return m_buffer.size() - m_writeIndex; } // 可写字节数
	size_t HeadIdleBytes() const { return m_readIndex; } // 前面读取完的空间

	void Append(const std::string& str);
	void Append(const char* data, size_t len);

	// 从socket buffer中读取数据
	ssize_t ReadFd(int fd);

	const char* peek() const { return begin() + m_readIndex; }

	void rm_ReadIndex(size_t len); // 读取

	std::string ReadAllAsString() { 
		return ReadAsString(ReadableBytes()); 
	}
	std::string ReadAsString(size_t len, bool onlySee = false) {
		std::string result(peek(), len);
		if(!onlySee) rm_ReadIndex(len);
		return result;
	}

	char* BeginWrite() { return begin() + m_writeIndex; }
	const char* BeginWrite() const { return begin() + m_writeIndex; }

	const char* FindCRLFCRLF() const {
		const char* target = "\r\n\r\n";
		const char* start = peek();
		const char* end = start + ReadableBytes();
		const char* result = std::search(start, end, target, target + 4);
		return (result == end) ? nullptr : result;
	}
private:
	char* begin() { return &*m_buffer.begin(); } // 获取m_buffer的地址
	const char* begin() const { return &*m_buffer.begin(); }
	void MakeSpace(size_t len); // 扩容
private:
	std::vector<char> m_buffer;
	static const size_t m_initSize = 1024; // 初始化大小
	size_t m_readIndex;
	size_t m_writeIndex;
};

