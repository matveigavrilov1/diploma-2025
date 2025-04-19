#pragma once

#include <atomic>
#include <thread>
#include <vector>
#include <functional>
#include "ts-queue.h"

namespace cs
{
class threadPool
{
public:
	explicit threadPool(size_t workersCount = 1);

	void run();
	void stop();

	using task_t = std::function<void()>;

	void pushTask(task_t&& task);
	void pushTask(const task_t& task);
	
	std::atomic<bool>& running();

private:
	void worker();

private:
	size_t workersCount_ { 1 };
	tsQueue<task_t> queue_;
	std::vector<std::jthread> workers_;
	std::atomic<bool> running_ { false };
};
} // namespace cs