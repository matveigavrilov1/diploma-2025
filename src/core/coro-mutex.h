#pragma once

#include <atomic>
#include <coroutine>

// #include "lf-queue.h"

#include "concurrentqueue.h"

namespace cs
{
class coroMutex
{
public:
	friend struct awaiter;

	struct awaiter
	{
		awaiter(coroMutex& cm, bool locked);

		bool await_ready();
		void await_suspend(std::coroutine_handle<> handle);
		void await_resume();

	private:
		coroMutex& cm_;
		bool locked_;
	};

	awaiter lock();
	void unlock();

	std::atomic<bool>& locked();
private:
	// lfQueue<std::coroutine_handle<>> queue_;
	moodycamel::ConcurrentQueue<std::coroutine_handle<>> queue_;
	std::atomic<bool> locked_ { false };
};
} // namespace cs