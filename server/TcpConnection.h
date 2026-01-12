#pragma once

#include "Callbacks.h"
#include "Buffer.h"
#include "Channel.h"
#include "EventLoop.h"
#include "InetAddress.h"
#include <any>

class TcpConnection : public std::enable_shared_from_this<TcpConnection>
{
public:
	TcpConnection(EventLoop* loop, int sock, const InetAddress& serverAddr, const InetAddress& clientAddr);
	~TcpConnection();
public:
	EventLoop* GetLoop() const { return m_loop; }

	const InetAddress& GetServerAddress() const { return m_sAddr; }
	const InetAddress& GetClientAddress() const { return m_cAddr; }

	bool IsConnected() const { return m_state == Connected; }

	void SendMessage(const std::string& message);
	void ShutDown(); // 主动关闭连接

	void SetConnectionCallback(const ConnectionCallback& cb) {
		m_connectionCb = cb;
	}
	void SetRecvMessageCallback(const RecvMessageCallback& cb) {
		m_messageCb = cb;
	}
	void SetWriteOverCallback(const WriteOverCallback& cb) {
		m_writeOverCb = cb;
	}
	void SetCloseCallback(const CloseCallback& cb) {
		m_closeCb = cb;
	}

	// 把channel注册到eventloop中
	void CreateConnect();
	// 把channel从eventloop中移除
	void DestroyConnect();

	void setContext(const std::any& context) { m_context = context; }
	const std::any& getContext() const { return m_context; }
	bool hasContext() const { return m_context.has_value(); }
private:
	enum STATE {
		DisConnected, // 未连接
		Connecting, // 正在连接
		Connected, // 已连接
		DisConnecting // 正在关闭连接
	};

	void SetState(STATE s) { m_state = s; }

	void HandleRead();
	void HandleWrite();
	void HandleClose();
	void HandleError();

	void SendInLoop(const std::string& message);
	void ShutdownInLoop();
private:
	EventLoop* m_loop;
	const int m_cliSock; // 服务的客户端socket
	const InetAddress m_sAddr; // 服务器地址
	const InetAddress m_cAddr; // 客户端地址

	std::any m_context; // 存用户名 string

	std::atomic<STATE> m_state;

	Buffer m_inputBuffer; // 接收缓冲区
	Buffer m_outputBuffer; // 发送缓冲区

	std::unique_ptr<Channel> m_channel;

	// 用户的回调
	ConnectionCallback m_connectionCb;
	RecvMessageCallback m_messageCb;
	WriteOverCallback m_writeOverCb;

	// 内部回调
	CloseCallback m_closeCb; // 用于通知TcpServer移除本连接
};

