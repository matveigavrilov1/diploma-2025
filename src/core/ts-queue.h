#pragma once
#include <queue>
#include <mutex>
#include <condition_variable>
#include <optional>
namespace cs
{

template<typename T>
class tsQueue
{
public:
	void push(const T& item) { emplace(item); }

	void push(T&& item) noexcept { emplace(std::move(item)); }

	template<typename... Args>
	void emplace(Args&&... args)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		queue_.emplace(std::forward<Args>(args)...);
		lock.unlock();
		cond_.notify_one();
	}

	bool pop(T& value)
	{
		std::lock_guard<std::mutex> lock(mutex_);
		if (queue_.empty())
		{
			return false;
		}
		value = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	bool wait_and_pop(T& value, std::atomic<bool>& running)
	{
		std::unique_lock<std::mutex> lock(mutex_);
		cond_.wait(lock, [this, &running]() { return !queue_.empty() || !running.load(); });

		if (!running.load())
		{
			return false;
		}

		value = std::move(queue_.front());
		queue_.pop();
		return true;
	}

	void notify_all() { cond_.notify_all(); }

private:
	std::queue<T> queue_;
	mutable std::mutex mutex_;
	mutable std::condition_variable cond_;
};
} // namespace cs