#pragma once

#include <atomic>
#include <vector>
#include <thread>
#include <functional>

#include "concurrentqueue.h" // Предполагается, что у вас есть потокобезопасная очередь

namespace cs
{

class threadPool
{
public:
	using task_t = std::function<void()>;

	explicit threadPool(size_t workersCount);

	void start();
	void stop() noexcept;

	void pushTask(task_t&& task);
	void pushTask(const task_t& task);

	std::atomic<bool>& running() { return running_; }

private:
	void worker(size_t thread_idx);


	size_t workersCount_;
	std::atomic<bool> running_ { false };
	std::vector<std::thread> workers_;
	std::vector<moodycamel::ConcurrentQueue<task_t>> queues_;
};

} // namespace cs