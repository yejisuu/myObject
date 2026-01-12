#pragma once

#include <map>
#include <vector>
#include <mutex>
#include <memory>
#include <functional>
#include <atomic>
#include <thread>

class Channel;

struct epoll_event;

class EventLoop
{
public:
	EventLoop();
	~EventLoop();
public:
	using Functor = std::function<void()>;
	using ChannelMap = std::map<int, Channel*>; // fd -> channel 的映射
public:
	// 启动事件循环
	void Loop();
	// 退出事件循环
	void Quit();

	// 更新epoll监听列表(标签)
	void UpdateChannel(Channel* channel);
	// 移除
	void RemoveChannel(Channel* channel);

	// 当前线程，立即执行cb，否则加入到队列
	void RunInLoop(Functor cb);
	// 放入队列
	void QueueInLoop(Functor cb);
	// 判断当前是否在事件循环的线程中
	bool IsInLoopThread() const { return m_threadId == std::this_thread::get_id(); }
private:
	// epoll_ctl的封装
	void Update(int operation, Channel* channel);
	// 唤醒loop
	void Wakeup();
	void HandleRead();

	// 执行任务队列
	void DoPendingFunctors();

	// 把有事件的Channel填入activeChannels
	void FillActiveChannels(int numEvents, epoll_event* events, std::vector<Channel*>* activeChannels) const;
private:
	int m_epollFd;
	ChannelMap m_channels;
	const std::thread::id m_threadId;
	std::atomic<bool> m_isLooping; // 循环是否在执行
	std::atomic<bool> m_isQuit; // 是否退出循环
	int m_wakeupFd; // 用于唤醒
	std::mutex m_mutex;
	std::unique_ptr<Channel> m_wakeupChannel; // 管理weakupFd
	std::vector<epoll_event> m_events; // epoll_wait返回的事件数组
	std::vector<Functor> m_pendingFunctors; // 待处理任务的队列
};

