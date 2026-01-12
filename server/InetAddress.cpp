#include "InetAddress.h"
#include <sys/socket.h>
#include <unistd.h>
#include <arpa/inet.h>
#include <string.h>

InetAddress::InetAddress(uint16_t port, bool loopbackOnly)
{
	memset(&m_addr, 0, sizeof m_addr); // «Â¡„
	m_addr.sin_family = AF_INET;
	m_addr.sin_addr.s_addr = htonl(loopbackOnly ? INADDR_LOOPBACK : INADDR_ANY);
	m_addr.sin_port = htons(port);
}

std::string InetAddress::ToIp() const
{
	char buf[64] = "";
	inet_ntop(AF_INET, &m_addr.sin_addr, buf, sizeof(buf));
	return buf;
}

std::string InetAddress::ToIpPort() const
{
	return ToIp() + ":" + std::to_string(ToPort());
}

uint16_t InetAddress::ToPort() const
{
	return ntohs(m_addr.sin_port);
}
