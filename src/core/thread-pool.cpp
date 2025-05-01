#include "thread-pool.h"

#include <random>

namespace cs
{
threadPool::threadPool(size_t workersCount)
: workersCount_(workersCount)
, queues_(workersCount)
, running_(false)
{
	workers_.reserve(workersCount_);
}

void threadPool::start()
{
	if (running_.exchange(true))
		return;

	for (size_t i = 0; i < workersCount_; ++i)
	{
		workers_.emplace_back([this, i]() { worker(i); });
	}
}

void threadPool::stop() noexcept
{
	if (!running_.exchange(false))
		return;

	for (auto& worker : workers_)
	{
		if (worker.joinable())
			worker.join();
	}

	workers_.clear();
	queues_.clear();
}

void threadPool::pushTask(task_t&& task)
{
	if (!running_.load(std::memory_order_relaxed))
		return;

	// Простой рандомный выбор очереди для балансировки
	static thread_local std::mt19937 generator(std::random_device {}());
	std::uniform_int_distribution<size_t> distribution(0, workersCount_ - 1);
	size_t index = distribution(generator);

	queues_[index].enqueue(std::move(task));
}

void threadPool::pushTask(const task_t& task)
{
	if (!running_.load(std::memory_order_relaxed))
		return;

	task_t copy = task;
	pushTask(std::move(copy));
}

void threadPool::worker(size_t thread_idx)
{
	auto& local_queue = queues_[thread_idx];

	while (running_.load(std::memory_order_relaxed))
	{
		task_t task;

		if (local_queue.try_dequeue(task))
		{
			task();
			continue;
		}

		static thread_local std::mt19937 generator(std::random_device {}());
		std::uniform_int_distribution<size_t> distribution(0, workersCount_ - 1);

		for (size_t i = 0; i < workersCount_ * 2; ++i)
		{
			size_t victim_idx = distribution(generator);
			if (victim_idx == thread_idx)
				continue;

			if (queues_[victim_idx].try_dequeue(task))
			{
				task();
				break;
			}
		}
	}
}

} // namespace cs