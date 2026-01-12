#pragma once

#include "InetAddress.h"

class Socket 
{
public:
	explicit Socket(int socket) : m_server(socket) {}
	~Socket();
public:
	int GetFd() const { return m_server; }

	void BindAddress(const InetAddress& localAddr);
	void Listen();

	int Accept(InetAddress* peerAddr);

	void SetReuseAddr(bool on);
	void SetNoDelay(bool on);
	void SetReusePort(bool on);

	static int CreateNonblockSocket();
private:
	const int m_server;
};
