#include "benchmark/coro.h"
#include <coroutine>

cs::task coroutine(cs::atomicCounterLogger& counter, size_t id, std::atomic<bool>& running, cs::coroMutex& mtx)
{
	while (running)
	{
		co_await mtx.lock();
		counter.increment(id);
		mtx.unlock();
		co_await std::suspend_always {};
	}
}

cs::task coroutine(cs::atomicCounterLogger& counter, size_t id, std::atomic<bool>& running, std::mutex& mtx)
{
	while (running)
	{
		std::lock_guard<std::mutex> lk { mtx };
		counter.increment(id);
		co_await std::suspend_always {};
	}
}