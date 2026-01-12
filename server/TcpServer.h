#pragma once

#include "Callbacks.h"
#include "TcpConnection.h"
#include "Acceptor.h"
#include "EventLoop.h"
#include "ThreadPool.h"
#include "UserManager.h"

class TcpServer
{
public:
	TcpServer(EventLoop* loop, const InetAddress& cliAddr, bool reusePort = false);
	~TcpServer();
public:
	void Start();
	// 在Start之前调用
	void SetThreadsNum(int numThreads);

	void SetRecvMessageCallback(const RecvMessageCallback& cb) {
		m_recvMessageCb = cb;
	}
	void SetConnectionCallback(const ConnectionCallback& cb) {
		m_connectionCb = cb;
	}
private:
	// 由Acceptor调用
	void NewConnection(int cliSock, const InetAddress& cliAddr);
	// 由TcpConnection调用
	void RemoveConnection(const TcpConnectionPtr& conn);
	// 由 EventLoop调用
	void RemoveConnectionInLoop(const TcpConnectionPtr& conn);
private:
	EventLoop* m_loop;

	ConnectionCallback m_connectionCb;
	RecvMessageCallback  m_recvMessageCb;

	std::unique_ptr<Acceptor> m_acceptor;

	std::shared_ptr<ThreadPool> m_threadPool;

	std::map<std::string, TcpConnectionPtr> m_connections;
	int m_connectionId;
};

