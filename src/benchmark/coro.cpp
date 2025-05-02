#include "benchmark/coro.h"
#include <chrono>
#include <thread>

cs::task coroutine(cs::atomicMultipleCounter& counter, size_t id, std::atomic<bool>& running, cs::coroMutex& mtx)
{
	while (running)
	{
		co_await mtx.lock();
		counter.increment(id);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		mtx.unlock();
		// co_await std::suspend_always {};
	}
}

cs::task coroutine(cs::atomicMultipleCounter& counter, size_t id, std::atomic<bool>& running, std::mutex& mtx)
{
	while (running)
	{
		mtx.lock();
		counter.increment(id);
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		mtx.unlock();
		// co_await std::suspend_always {};
	}
	co_return;
}