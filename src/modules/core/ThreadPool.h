/**
 * @file
 */

/*
 Copyright (c) 2012 Jakob Progsch, Václav Zeman

 This software is provided 'as-is', without any express or implied
 warranty. In no event will the authors be held liable for any damages
 arising from the use of this software.

 Permission is granted to anyone to use this software for any purpose,
 including commercial applications, and to alter it and redistribute it
 freely, subject to the following restrictions:

 1. The origin of this software must not be misrepresented; you must not
 claim that you wrote the original software. If you use this software
 in a product, an acknowledgment in the product documentation would be
 appreciated but is not required.

 2. Altered source versions must be plainly marked as such, and must not be
 misrepresented as being the original software.

 3. This notice may not be removed or altered from any source
 distribution.
 */

#pragma once

#include <vector>
#include <queue>
#include <memory>
#include <thread>
#include <mutex>
#include <condition_variable>
#include <atomic>
#include <future>
#include <functional>

namespace core {

class ThreadPool final {
public:
	explicit ThreadPool(size_t, const char *name = nullptr);
	~ThreadPool();

	/**
	 * Enqueue functors or lambdas into the thread pool
	 */
	template<class F, class ... Args>
	auto enqueue(F&& f, Args&&... args) -> std::future<typename std::result_of<F(Args...)>::type>;

	size_t size() const;
	void init();
	void shutdown(bool wait = false);
private:
	const size_t _threads;
	const char *_name;
	// need to keep track of threads so we can join them
	std::vector<std::thread> _workers;
	// the task queue
	std::queue<std::function<void()> > _tasks;

	// synchronization
	std::mutex _queueMutex;
	std::condition_variable _queueCondition;
	std::mutex _shutdownMutex;
	std::condition_variable _shutdownCondition;
	std::atomic_int _shutdownCount { 0 };
	std::atomic_bool _stop { false };
	std::atomic_bool _force { false };
};

// add new work item to the pool
template<class F, class ... Args>
auto ThreadPool::enqueue(F&& f, Args&&... args)
-> std::future<typename std::result_of<F(Args...)>::type> {
	using return_type = typename std::result_of<F(Args...)>::type;
	if (_stop) {
		return std::future<return_type>();
	}

	auto task = std::make_shared<std::packaged_task<return_type()> >(std::bind(std::forward<F>(f), std::forward<Args>(args)...));

	std::future<return_type> res = task->get_future();
	{
		std::unique_lock<std::mutex> lock(_queueMutex);
		if (_stop) {
			return std::future<return_type>();
		}
		_tasks.emplace([task]() {(*task)();});
	}
	_queueCondition.notify_one();
	return res;
}

inline size_t ThreadPool::size() const {
	return _threads;
}

}
