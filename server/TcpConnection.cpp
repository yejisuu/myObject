#include "TcpConnection.h"
#include <unistd.h>
#include <iostream>
#include <cassert>
#include <string.h>
#include "UserManager.h"

TcpConnection::TcpConnection(EventLoop* loop, int sock, const InetAddress& serverAddr, const InetAddress& clientAddr)
	:m_loop(loop), m_cliSock(sock), m_sAddr(serverAddr), m_cAddr(clientAddr), m_state(Connecting), m_channel(new Channel(loop, sock))
{
	m_channel->SetReadCallback(std::bind(&TcpConnection::HandleRead, this));
	m_channel->SetWriteCallback(std::bind(&TcpConnection::HandleWrite, this));
	m_channel->SetCloseCallback(std::bind(&TcpConnection::HandleClose, this));
	m_channel->SetErrorCallback(std::bind(&TcpConnection::HandleError, this));
}

TcpConnection::~TcpConnection()
{
	close(m_cliSock);
}

void TcpConnection::SendMessage(const std::string& message)
{
	if (m_state != Connected) return;

	// 如果是在当前线程
	if (m_loop->IsInLoopThread()) {
		SendInLoop(message);
	}
	else {
		// 如果是线程池调用的，打包成任务，由EventLoop执行
		m_loop->RunInLoop(std::bind(&TcpConnection::SendInLoop, this, message));
	}
}

void TcpConnection::ShutDown()
{
	if (m_state == Connected) {
		SetState(DisConnecting);
		m_loop->RunInLoop(std::bind(&TcpConnection::ShutdownInLoop, this));
	}
}

void TcpConnection::HandleRead() 
{
	ssize_t len = m_inputBuffer.ReadFd(m_cliSock);
	if (len > 0) { // 读到数据
		if (m_messageCb) {
			m_messageCb(shared_from_this(), &m_inputBuffer);
		}
	}
	else if (len == 0) {
		HandleClose();
	}
	else {
		if (errno == EAGAIN || errno == EWOULDBLOCK) {
			return; // 只是暂时没数据，不是错误
		}
		perror("read error"); // 打印真正的错误原因
		HandleError();
	}
}

void TcpConnection::HandleWrite()
{
	if (m_channel->IsWriting()) {
		ssize_t len = write(m_cliSock, m_outputBuffer.peek(), m_outputBuffer.ReadableBytes()); // 发送缓冲区的可读数据
		if (len > 0) {
			m_outputBuffer.rm_ReadIndex(len); // 发送成功，从缓冲区中移除
			if (m_outputBuffer.ReadableBytes() == 0) {
				// 告诉EventLoop,不在关注写事件
				m_channel->DisableWriting();

				if (m_writeOverCb) {
					m_writeOverCb(shared_from_this());
				}
				if (m_state == DisConnecting) {
					ShutdownInLoop();
				}
			}
		}
		else {
			std::cout << "TcpConnection::HandleWrite() write error" << std::endl;
		}
	}
}

void TcpConnection::HandleClose()
{
	SetState(DisConnected);
	m_channel->DisableAll();

	// 创建一个shared_ptr保护自己，防止回调时自己被摧毁
	TcpConnectionPtr guardThis(shared_from_this());

	if (m_connectionCb) {
		m_connectionCb(guardThis);
	}
	if (m_closeCb) {
		m_closeCb(guardThis); // TcpServer的移除连接回调
	}
	UserManager::getInstance().RemoveConnection(shared_from_this());
}

void TcpConnection::HandleError()
{
	int err;
	socklen_t len = sizeof err;
	if (getsockopt(m_cliSock, SOL_SOCKET, SO_ERROR, &err, &len) < 0) {
		err = errno;
	}
	std::cerr << "TcpConnection::HandleError() [" << m_cAddr.ToIpPort() << "] - " << strerror(err) << std::endl;
}

void TcpConnection::SendInLoop(const std::string& message)
{
	ssize_t send_len = 0;
	ssize_t remaining = message.size(); // 剩余的
	bool faultError = false;
	
	if (m_outputBuffer.ReadableBytes() == 0) { // 如果缓冲区为空
		// 直接发送现在要发的
		send_len = write(m_cliSock, message.data(), message.size());
		if (send_len >= 0) {
			remaining = message.size() - send_len;
			if (remaining == 0 && m_writeOverCb) {
				// 如果一次性发完了，调用发送完毕的回调
				m_loop->QueueInLoop(std::bind(m_writeOverCb, shared_from_this()));
			}
		}
		else {
			send_len = 0;
			if (errno != EWOULDBLOCK) { // 如果不是缓冲区满导致的
				std::cerr << "TcpConnection::SendInLoop() write error" << std::endl;
				faultError = true;
			}
		}
	}
	if (!faultError && remaining > 0) {

		m_outputBuffer.Append(message.data() + send_len, remaining);


		if (!m_channel->IsWriting()) {
			m_channel->EnableWriting();
		}
	}

	struct sockaddr_in peerAddr;
	socklen_t peerAddrLen = sizeof(peerAddr);
	if (getpeername(m_cliSock, (struct sockaddr*)&peerAddr, &peerAddrLen) == 0) {
		int peerPort = ntohs(peerAddr.sin_port);

		std::cout << "[Server Debug] fd=" << m_cliSock << " port is : " << peerPort << std::endl;

		
	}
}

void TcpConnection::ShutdownInLoop()
{
	// 只有没数据可写时，才会关闭写端
	if (m_outputBuffer.ReadableBytes() == 0) {
		::shutdown(m_cliSock, SHUT_WR);
	}
	// 如果数据没发完，HandleWrite会在发完后检查状态，并调用ShutdownInLoop
}

void TcpConnection::CreateConnect()
{
	SetState(Connected);
	m_channel->Tie(shared_from_this());

	m_channel->EnableReading(); // 开始监听读事件

	// 设置用户设置的连接建立回调
	if (m_connectionCb) {
		m_connectionCb(shared_from_this());
	}
}

void TcpConnection::DestroyConnect()
{
	if (m_state == Connected) {
		SetState(DisConnected);
		m_channel->DisableAll(); // 从EventLoop中移除所有监听
	}
	m_channel->RemoveFormEventLoop();
}