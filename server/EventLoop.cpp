#include "EventLoop.h"
#include <sys/eventfd.h>
#include <iostream>
#include <unistd.h>
#include "Channel.h"
#include <sys/epoll.h>
#include "TcpConnection.h"

int CreateEventFd() {
	int evtFd = eventfd(0, EFD_NONBLOCK | EFD_CLOEXEC);
	if (evtFd < 0) {
		std::cerr << "eventfd failed" << std::endl;
		abort(); // 异常终止进程
	}
	return evtFd;
}

EventLoop::EventLoop():m_isLooping(false), m_isQuit(false), m_threadId(std::this_thread::get_id()),
		m_epollFd(epoll_create1(EPOLL_CLOEXEC)), m_events(16), m_wakeupFd(CreateEventFd()),
		m_wakeupChannel(new Channel(this, m_wakeupFd))
{
	if (m_epollFd < 0) {
		std::cerr << "epoll_create1 failed" << std::endl;
		abort();
	}
	// 可读时调用HandleRead
	m_wakeupChannel->SetReadCallback(std::bind(&EventLoop::HandleRead, this));
	// 始终监听 wakeupFd 上的读事件
	m_wakeupChannel->EnableReading();
}

EventLoop::~EventLoop()
{
	close(m_epollFd);
	close(m_wakeupFd);
}

void EventLoop::Loop()
{
	m_isLooping = true;
	m_isQuit = false;

	while (!m_isQuit) {
		std::vector<Channel*> activeChannels;
		int numEvents = epoll_wait(m_epollFd, &*m_events.begin(), static_cast<int>(m_events.size()), -1); // -1 无限等待
		if (numEvents > 0) {
			FillActiveChannels(numEvents, &*m_events.begin(), &activeChannels); // 把事件注册到channel里

			if (static_cast<size_t>(numEvents) == m_events.size()) {
				m_events.resize(m_events.size() * 2);
			}
		}
		else if (numEvents == 0) { // 无事件发生

		}
		else {
			if (errno != EINTR) { // EINTR是信号中断，不算出错，其他都是出错
				std::cerr << "epoll_wait error" << std::endl;
			}
		}
		// 批量处理事件
		for (Channel* channel : activeChannels) {
			channel->HandleEvent(); // 每个channel执行各自的回调函数
		}

		// 执行跨线程任务队列
		DoPendingFunctors();
	}
	m_isLooping = false;
}

void EventLoop::Quit()
{
	m_isQuit = true;
	if (!IsInLoopThread()) {
		Wakeup();
	}
}

void EventLoop::UpdateChannel(Channel* channel)
{
	int fd = channel->GetFd();
	if (channel->GetIndex() == -1) {
		m_channels[fd] = channel; // map映射
		channel->SetIndex(1);
		Update(EPOLL_CTL_ADD, channel);
	}
	else { // MOD or DEL
		if (channel->IsNoneEvent()) {
			Update(EPOLL_CTL_DEL, channel);
			channel->SetIndex(2);
		}
		else {
			Update(EPOLL_CTL_MOD, channel);
		}
	}
}

void EventLoop::RemoveChannel(Channel* channel)
{
	int fd = channel->GetFd();
	if (channel->GetIndex() == 1) {
		Update(EPOLL_CTL_DEL, channel);
	}
	channel->SetIndex(-1);
	m_channels.erase(fd); // map中删除这个映射
}

void EventLoop::RunInLoop(Functor cb)
{
	if (IsInLoopThread()) {
		cb(); // 如果是当前线程，立即执行回调
	}
	else {
		QueueInLoop(cb); // 否则加入任务队列
	}
}

void EventLoop::QueueInLoop(Functor cb)
{
	{
		std::lock_guard<std::mutex> lock(m_mutex); // 局部锁
		m_pendingFunctors.push_back(std::move(cb)); // 把任务加入到任务队列中
	}
	if (!IsInLoopThread() || m_isQuit) {
		Wakeup();
	}
}

void EventLoop::Update(int operation, Channel* channel)
{
	epoll_event event;
	event.events = channel->GetEvents();
	event.data.ptr = channel; // channel存入epoll, 事件发生时能取回
	int fd = channel->GetFd();
	if (epoll_ctl(m_epollFd, operation, fd, &event) < 0) {
		std::cerr << "epoll_ctl() error  op=" << operation << "fd=" << fd << std::endl;
	}
}

void EventLoop::Wakeup()
{
	uint64_t one = 1;
	ssize_t n = write(m_wakeupFd, &one, sizeof one);
	if (n != sizeof one) {
		std::cerr << "Wakeup() writes " << n << " bytes" << std::endl;
	}
}



void EventLoop::HandleRead()
{
	uint64_t one = 1;
	ssize_t n = read(m_wakeupFd, &one, sizeof one);
	if (n != sizeof one) {
		std::cerr << "HandleRead() reads " << n << " bytes" << std::endl;
	}
}

void EventLoop::DoPendingFunctors()
{
	std::vector<Functor> functors;
	{
		std::lock_guard<std::mutex> lock(m_mutex); // 局部锁
		functors.swap(m_pendingFunctors);
	}
	for (const Functor& functor : functors) {
		functor();
	}
}

void EventLoop::FillActiveChannels(int numEvents, epoll_event* events, std::vector<Channel*>* activeChannels) const
{
	for (int i = 0; i < numEvents; i++) {
		// 取回channel*
		Channel* channel = static_cast<Channel*>(events[i].data.ptr);
		// 设置channel实际发生的事件
		channel->SetRevents(events[i].events);
		activeChannels->push_back(channel);
	}
}
