#include "Acceptor.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <iostream>
#include <unistd.h>

Acceptor::Acceptor(EventLoop* loop, const InetAddress& listenAddr, bool reusePort)
	: m_loop(loop), m_accSock(Socket::CreateNonblockSocket()), m_accChannel(loop, m_accSock.GetFd())
	, m_idleFd(open("/dev/null", O_RDONLY | O_CLOEXEC)) // 打开一个空闲fd
{
	m_accSock.SetReuseAddr(true);
	m_accSock.SetNoDelay(true);
	m_accSock.SetReusePort(reusePort);
	m_accSock.BindAddress(listenAddr);
	// 设置channel的读回调
	m_accChannel.SetReadCallback(std::bind(&Acceptor::HandleRead, this));
}

Acceptor::~Acceptor()
{
	m_accChannel.DisableAll();
	m_accChannel.RemoveFormEventLoop();
	close(m_idleFd);
}

void Acceptor::Listen() {
	m_accSock.Listen();
	m_accChannel.EnableReading();
}

void Acceptor::HandleRead() {
	InetAddress cliAddr;

	int cli_sock = m_accSock.Accept(&cliAddr);
	while (cli_sock >= 0) {
		if (m_newConnectionCb) {
			// 把这个新连接的fd和addr交给上层TcpServer
			m_newConnectionCb(cli_sock, cliAddr);
		}
		else {
			close(cli_sock);
		}
		// 继续accept
		cli_sock = m_accSock.Accept(&cliAddr);
	}
	if (errno == EMFILE) { // accept失败 文件描述符用完了
		close(m_idleFd);
		m_idleFd = accept(m_accSock.GetFd(), NULL, NULL);
		close(m_idleFd);
		m_idleFd = open("/dev/null", O_RDONLY | O_CLOEXEC);
		std::cerr << "Acceptor::HandleRead() error - accept EMFILE" << std::endl;
	}
}