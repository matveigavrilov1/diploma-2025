#include "benchmark/coro.h"

#include <thread>
#include <chrono>

#include "core/task-manager.h"

#include <spdlog/spdlog.h>

// cs::task coroutine(cs::atomicMultipleCounter& counter, size_t id, std::atomic<bool>& running, cs::coroMutex& mtx, size_t counterIdx)
// {
// 	spdlog::debug("Coro [{}] starting", id);
// 	while (running)
// 	{
// 		spdlog::debug("Coro [{}] locking coroMutex", id);
// 		co_await mtx.lock();
// 		spdlog::debug("Coro [{}] incrementing counter {}", id, counterIdx);
// 		counter.increment(counterIdx);
// 		std::this_thread::sleep_for(std::chrono::milliseconds(1));
// 		spdlog::debug("Coro [{}] unlocking coroMutex", id);
// 		mtx.unlock();
// 		spdlog::debug("Coro [{}] waiting", id);
// 		// co_await std::suspend_always {};
// 	}
// 	spdlog::debug("Coro [{}] finishing", id);
// }

// cs::task coroutine(cs::atomicMultipleCounter& counter, size_t id, std::atomic<bool>& running, std::mutex& mtx, size_t counterIdx)
// {
// 	spdlog::debug("Coro [{}] starting", id);
// 	while (running)
// 	{
// 		spdlog::debug("Coro [{}] locking std::mutex", id);
// 		mtx.lock();
// 		spdlog::debug("Coro [{}] incrementing counter {}", id, counterIdx);
// 		counter.increment(counterIdx);
// 		std::this_thread::sleep_for(std::chrono::milliseconds(1));
// 		spdlog::debug("Coro [{}] unlocking std::mutex", id);
// 		mtx.unlock();
// 		spdlog::debug("Coro [{}] waiting", id);
// 		// co_await std::suspend_always {};
// 	}
// 	spdlog::debug("Coro [{}] finishing", id);
// 	co_return;
// }

cs::task coroutine(cs::atomicMultipleCounter& counter, size_t id, std::atomic<bool>& running, cs::coroMutex& mtx, size_t counterIdx)
{
	if (!running)
		co_return;
	co_await mtx.lock();
	counter.increment(counterIdx);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	mtx.unlock();
	cs::taskManager::instance().execute(coroutine(counter, id, running, mtx, counterIdx));
}

cs::task coroutine(cs::atomicMultipleCounter& counter, size_t id, std::atomic<bool>& running, std::mutex& mtx, size_t counterIdx)
{
	if (!running)
		co_return;
	mtx.lock();
	counter.increment(counterIdx);
	std::this_thread::sleep_for(std::chrono::milliseconds(1));
	mtx.unlock();
	cs::taskManager::instance().execute(coroutine(counter, id, running, mtx, counterIdx));
}