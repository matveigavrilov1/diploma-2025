#include "thread-pool.h"

namespace cs
{
threadPool::threadPool(size_t workersCount)
: workersCount_(workersCount)
{ }

void threadPool::run()
{
	workers_.clear();
	running_ = true;
	workers_.reserve(workersCount_);
	for (size_t i = 0; i < workersCount_; ++i)
	{
		workers_.emplace_back([this]() { this->worker(); });
	}
}

void threadPool::stop()
{
	running_ = false;
	queue_.notify_all();
	for (auto& worker : workers_)
	{
		worker.join();
	}
}

void threadPool::pushTask(task_t&& task)
{
	if (running_)
		queue_.push(std::move(task));
}

void threadPool::pushTask(const task_t& task)
{
	if (running_)
		queue_.push(task);
}

void threadPool::worker()
{
	while (running_)
	{
		task_t task;
		if (queue_.wait_and_pop(task, running_))
		    task();
	}
}

} // namespace cs