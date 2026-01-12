#pragma once

#include <string.h>
#include <string>
#include <netinet/in.h>

class InetAddress 
{
public:
	InetAddress() { memset(&m_addr, 0, sizeof m_addr); }
	explicit InetAddress(uint16_t port, bool loopbackOnly = false);
	explicit InetAddress(const struct sockaddr_in& addr) : m_addr(addr) {}
public:
	std::string ToIp() const;
	std::string ToIpPort() const;
	uint16_t ToPort() const;

	const struct sockaddr* GetSockAddr() const {
		return (const struct sockaddr*)(&m_addr);
	}
	void SetSockAddr(const struct sockaddr_in& addr) { m_addr = addr; }
private:
	struct sockaddr_in m_addr;
};
