#include "Buffer.h"
#include <sys/uio.h>
#include <iostream>

void Buffer::Append(const std::string& str)
{
	Append(str.data(), str.size());
}

void Buffer::Append(const char* data, size_t len)
{
	// 如果剩余控件不够，就扩容
	if (WriteableBytes() < len) {
		MakeSpace(len);
	}
	std::copy(data, data + len, BeginWrite()); // 把数据拷贝到m_buffer里
	m_writeIndex += len;
}

// 普通的 read 如果缓冲区不够大，数据就丢了或者要读很多次。 Muduo使用了readv(分散读)，它准备了额外内存(extraBuf)
ssize_t Buffer::ReadFd(int fd)
{
	char extraBuf[65536]; // 额外内存 64K
	struct iovec vec[2];
	const size_t writeable = WriteableBytes();

	// m_buffer 内部的空闲区
	vec[0].iov_base = begin() + m_writeIndex; // 读取数据的起始位置
	vec[0].iov_len = writeable;
	// 额外内存
	vec[1].iov_base = extraBuf;
	vec[1].iov_len = sizeof extraBuf;
	// 判断使用几个缓冲区
	const int iovcont = (writeable < sizeof extraBuf) ? 2 : 1; 

	const ssize_t len = readv(fd, vec, iovcont);
	if (len < 0) {
		std::cout << "readv error" << std::endl;
	}
	else if (static_cast<size_t>(len) <= writeable) { // 全部读取到m_buffer缓冲区了
		m_writeIndex += len;
	}
	else { // extraBuf也读取到了数据
		m_writeIndex = m_buffer.size();
		Append(extraBuf, len - writeable);
	}
	return len;
}

void Buffer::rm_ReadIndex(size_t len)
{
	if (len < ReadableBytes()) {
		m_readIndex += len;
	}
	else {
		m_readIndex = 0;
		m_writeIndex = 0;
	}
}

void Buffer::MakeSpace(size_t len)
{
	if (WriteableBytes() + HeadIdleBytes() < len) {
		m_buffer.resize(m_writeIndex + len);
	}
	else {
		// 前面的+后面可用的空间够用，就把数据往前挪
		size_t dataSize = ReadableBytes();
		std::copy(begin() + m_readIndex, begin() + m_writeIndex, begin());
		m_readIndex = 0;
		m_writeIndex = dataSize;
	}
}




