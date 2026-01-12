#include "Channel.h"
#include "EventLoop.h"
#include <sys/epoll.h>
#include <iostream>

const int Channel::NoneEvent = 0;
const int Channel::ReadEvent = EPOLLIN | EPOLLPRI;
const int Channel::WriteEvent = EPOLLOUT;

Channel::Channel(EventLoop* loop, int fd) : m_loop(loop), m_fd(fd), m_events(0), m_revents(0), m_index(-1)
{

}

Channel::~Channel()
{

}

void Channel::handleEventWithGuard()
{
	// 对端关闭，且没有读事件，就close
	if ((m_revents & EPOLLHUP) && !(m_revents & EPOLLIN)) {
		if (closeCallback) {
			closeCallback();
		}
	}
	// 处理错误事件
	if (m_revents & EPOLLERR) {
		if (errorCallback) {
			errorCallback();
		}
	}
	// 处理读事件
	if (m_revents & (EPOLLIN | EPOLLPRI)) {
		if (readCallback) {
			readCallback();
		}
	}
	// 处理写事件
	if (m_revents & (EPOLLOUT)) {
		if (writeCallback) {
			writeCallback();
		}
	}
}

void Channel::HandleEvent()
{
	if (m_isTied) {
		// 尝试将weak_ptr提升为shared_ptr
		std::shared_ptr<void> guard = m_tie.lock(); 
		if (guard) { // 成功，代表TcpConnection还活着
			handleEventWithGuard();
		}
		else {
			// 提升失败，说明TcoConnection销毁了，什么也不做
		}
	}
	else {
		// 未绑定，直接处理
		handleEventWithGuard();
	}

}

void Channel::RemoveFormEventLoop()
{
	m_loop->RemoveChannel(this);
}

void Channel::Update()
{
	m_loop->UpdateChannel(this);
}


