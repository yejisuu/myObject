#pragma once

#include <functional>
#include <memory>

class EventLoop;

// 事件标签
class Channel
{
public:
	Channel(EventLoop* loop, int fd);
	~Channel();
public:
	using EventCallback = std::function<void()>;
public:
	void handleEventWithGuard(); // 防止TcpConnection析构了，但是channel还在回调
	void HandleEvent();

	void SetReadCallback(const EventCallback& cb) { readCallback = cb; }
	void SetWriteCallback(const EventCallback& cb) { writeCallback = cb; }
	void SetCloseCallback(const EventCallback& cb) { closeCallback = cb; }
	void SetErrorCallback(const EventCallback& cb) { errorCallback = cb; }

	int GetFd() const { return m_fd; }
	int GetEvents() const { return m_events; }
	void SetRevents(int revents) { m_revents = revents; }
	bool IsNoneEvent() const { return m_events == NoneEvent; }

	void Tie(const std::shared_ptr<void>& obj) {
		m_tie = obj;
		m_isTied = true; // 绑定了
	}

	// 更新监听事件
	void EnableReading() { m_events |= ReadEvent; Update(); }
	void DisableReading() { m_events &= ~ReadEvent; Update(); }
	void EnableWriting() { m_events |= WriteEvent; Update(); }
	void DisableWriting() { m_events &= ~WriteEvent; Update(); }
	// 移除所有监听事件
	void DisableAll() { m_events = NoneEvent; Update(); }

	bool IsReading() const { return m_events & ReadEvent; }
	bool IsWriting() const { return m_events & WriteEvent; }
	
	// -1 未加入epoll, 1 已加入, 2 已移除
	int GetIndex() const { return m_index; }
	void SetIndex(int idx) { m_index = idx; }

	EventLoop* ownerLoop() { return m_loop; }

	// EventLoop删除这个channel
	void RemoveFormEventLoop();
private:
	// 更新epoll监听
	void Update();
private:
	EventCallback readCallback;
	EventCallback writeCallback;
	EventCallback closeCallback;
	EventCallback errorCallback;

	EventLoop* m_loop;
	const int m_fd; // channel负责的fd
	int m_events; // 监听的事件
	int m_revents; // epoll_wait返回的实际事件
	int m_index;

	static const int NoneEvent;
	static const int ReadEvent; // EPOLLPRI 高优先级数据
	static const int WriteEvent; 

	std::weak_ptr<void> m_tie;
	bool m_isTied; // 是否绑定TcpConnection
};

