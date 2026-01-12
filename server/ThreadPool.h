#pragma once

#include <vector>
#include <queue>
#include <thread>
#include <functional>
#include <mutex>
#include <condition_variable>
#include <future>

class ThreadPool {
public:
	ThreadPool(size_t threads) :m_isRun(true) {
		if (threads == 0) {
			threads = std::thread::hardware_concurrency(); // 自动设置成cpu核心数
		}
		for (size_t i = 0; i < threads; i++) {
			// 创建工作线程
			m_threads.emplace_back([this] {
				// 工作线程循环
				for (;;) {
					std::function<void()> task;
					
					{
						std::unique_lock<std::mutex> lock(this->m_mutex);

						// 如果线程池没停，或者任务队列不空，线程就等待在这里
						this->cv.wait(lock, [this] {return !this->m_isRun || !this->m_tasks.empty(); });

						// 如果线程池停止或者任务队列空了，就退出线程
						if (!this->m_isRun && this->m_tasks.empty()) {
							return;
						}
						// 获取一个任务
						task = std::move(this->m_tasks.front());
						this->m_tasks.pop();
					}
					task();
				}
			});
		}
	}

	~ThreadPool() {
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			m_isRun = false;
		}
		cv.notify_all(); // 唤醒所有在睡觉的线程

		// 等待所有线程执行完毕并退出
		for (std::thread& thread : m_threads) {
			thread.join();
		}
	}
public:
	template<class F, class... Args>
	auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type> {
		using return_type = typename std::result_of<F(Args...)>::type;

		auto task = std::make_shared<std::packaged_task<return_type()>>(
			std::bind(std::forward<F>(f), std::forward<Args>(args)...)
		);
		std::future<return_type> res = task->get_future();
		
		{
			std::unique_lock<std::mutex> lock(m_mutex);
			if (!m_isRun)
				throw std::runtime_error("enqueue on stopped ThreadPool");
			m_tasks.emplace([task]() { (*task)(); });
		}
		cv.notify_one();
		return res;
	}
private:
	std::vector<std::thread> m_threads;
	std::queue<std::function<void()>> m_tasks; // 任务队列
	std::mutex m_mutex;
	std::condition_variable cv; // 条件变量，用于线程的等待和唤醒
	bool m_isRun; 
};
