#include "Socket.h"
#include <unistd.h>
#include <iostream>
#include "Acceptor.h"
#include <netinet/tcp.h>

Socket::~Socket()
{
	close(m_server);
}

void Socket::BindAddress(const InetAddress& localAddr)
{
	if (bind(m_server, localAddr.GetSockAddr(), sizeof(struct sockaddr_in)) < 0) {
		std::cerr << "BindAddress() bind error" << std::endl;
		abort();
	}
}

void Socket::Listen()
{
	if (listen(m_server, 5) < 0) {
		std::cerr << "Listen() listen error" << std::endl;
		abort();
	}
}

int Socket::Accept(InetAddress* peerAddr)
{
	struct sockaddr_in cli_addr;
	socklen_t cli_addr_len = sizeof cli_addr;
	int cli_sock = accept4(m_server, (struct sockaddr*)&cli_addr, &cli_addr_len, SOCK_NONBLOCK | SOCK_CLOEXEC);
	if (cli_sock >= 0) {
		peerAddr->SetSockAddr(cli_addr);
	}
	return cli_sock;
}

void Socket::SetReuseAddr(bool on)
{
	int optval = on ? 1 : 0;
	setsockopt(m_server, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));
	
}

void Socket::SetNoDelay(bool on) {
	int optval = on ? 1 : 0;
	setsockopt(m_server, IPPROTO_TCP, TCP_NODELAY, &optval, sizeof(optval));
}

void Socket::SetReusePort(bool on)
{
	int optval = on ? 1 : 0;
	setsockopt(m_server, SOL_SOCKET, SO_REUSEPORT, &optval, sizeof(optval));
}

int Socket::CreateNonblockSocket()
{
	int sockfd = socket(AF_INET, SOCK_STREAM | SOCK_NONBLOCK | SOCK_CLOEXEC, 0);
	if (sockfd < 0) {
		std::cerr << "CreateNonblockSocket() socket error" << std::endl;
		abort();
	}
	return sockfd;
}
