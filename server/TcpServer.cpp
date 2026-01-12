#include "TcpServer.h"

TcpServer::TcpServer(EventLoop* loop, const InetAddress& cliAddr, bool reusePort)
	: m_loop(loop), m_acceptor(new Acceptor(loop, cliAddr, reusePort))
	, m_threadPool(std::make_shared<ThreadPool>(0)) // 初始化0个线程
	, m_connectionId(1)
{
	m_acceptor->SetNewConnectionCallback(std::bind(&TcpServer::NewConnection, this, std::placeholders::_1, std::placeholders::_2));
}

TcpServer::~TcpServer()
{
	// 确保所有连接都被移除
	for (auto& item : m_connections) {
		TcpConnectionPtr conn = item.second;
		item.second.reset(); // 断开shared_ptr
		m_loop->RunInLoop(std::bind(&TcpConnection::DestroyConnect, conn));
	}
}

void TcpServer::Start()
{
	//if (m_threadPool) m_threadPool->Start();
	m_loop->RunInLoop(std::bind(&Acceptor::Listen, m_acceptor.get()));
}

void TcpServer::SetThreadsNum(int numThreads)
{
	m_threadPool = std::make_shared<ThreadPool>(numThreads);
}

void TcpServer::NewConnection(int cliSock, const InetAddress& cliAddr)
{
	std::string cliName = cliAddr.ToIpPort() + "#" + std::to_string(m_connectionId++);

	InetAddress localAddr(0);
	TcpConnectionPtr conn = std::make_shared<TcpConnection>(m_loop, cliSock, localAddr, cliAddr);
	
	m_connections[cliName] = conn; // TcpServer记录这个客户端
	std::cout << "user[" << cliName << "]is ADD. Current Size:" << m_connections.size() << std::endl;
	conn->SetConnectionCallback(m_connectionCb);
	conn->SetRecvMessageCallback(m_recvMessageCb);
	conn->SetCloseCallback(std::bind(&TcpServer::RemoveConnection, this, std::placeholders::_1));

	// 注册epoll
	m_loop->RunInLoop(std::bind(&TcpConnection::CreateConnect, conn));
}

void TcpServer::RemoveConnection(const TcpConnectionPtr& conn)
{
	m_loop->RunInLoop(std::bind(&TcpServer::RemoveConnectionInLoop, this, conn));
}

void TcpServer::RemoveConnectionInLoop(const TcpConnectionPtr& conn)
{
	for (auto it = m_connections.begin(); it != m_connections.end(); ++it) {
		if (it->second == conn) {
			// 找到了，删除
			std::cout << "user[" << it->first << "]is DEL" << std::endl;
			m_connections.erase(it);
			break;
		}
	}
	m_loop->QueueInLoop(std::bind(&TcpConnection::DestroyConnect, conn));
	// 业务层移除
	UserManager::getInstance().RemoveConnection(conn);
}

