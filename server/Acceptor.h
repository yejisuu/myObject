#pragma once

#include "Socket.h"
#include "Channel.h"
#include <fcntl.h>


class EventLoop;
class InetAddress;

class Acceptor
{
public:
	Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort);
	~Acceptor();
public:
	using NewConnectionCallback = std::function<void(int sock, const InetAddress&)>;
public:
	void SetNewConnectionCallback(const NewConnectionCallback& cb) {
		m_newConnectionCb = cb;
	}
	void Listen();
private:
	void HandleRead(); // m_accChannel的读回调函数
private:
	EventLoop* m_loop;
	Socket m_accSock;
	Channel m_accChannel;
	NewConnectionCallback m_newConnectionCb; // 新连接到来时需要执行的回调
	
	int m_idleFd;
};

