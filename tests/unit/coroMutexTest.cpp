#include <gtest/gtest.h>

#include "core/coro-mutex.h"
#include "core/task-manager.h"
#include "core/coro-mutex.h"

#include <atomic>
#include <thread>
#include <chrono>

using namespace cs;

TEST(CoroMutexTest, BasicLock)
{
	coroMutex mtx;
	auto awaiter = mtx.lock();
	EXPECT_TRUE(mtx.locked());
}

TEST(CoroMutexTest, DoubleBasicLock)
{
	coroMutex mtx;
	auto awaiter = mtx.lock();
	EXPECT_TRUE(mtx.locked());
	auto awaiter2 = mtx.lock();
	EXPECT_TRUE(mtx.locked());
}

TEST(CoroMutexTest, BasicUnlock)
{
	coroMutex mtx;
	auto awaiter = mtx.lock();
	EXPECT_TRUE(mtx.locked());
	mtx.unlock();
	EXPECT_FALSE(mtx.locked());
}

TEST(CoroMutexTest, AwaiterReady)
{
	coroMutex mtx;

	auto awaiter1 = mtx.lock();
	EXPECT_TRUE(awaiter1.await_ready());

	auto awaiter2 = mtx.lock();
	EXPECT_FALSE(awaiter2.await_ready());

	mtx.unlock();
}

TEST(CoroMutexTest, BasicLockAndUnlockByCoro)
{
	coroMutex mtx;
	int counter = 0;

	auto tp = std::make_shared<threadPool>(1);
	taskManager::instance().init(tp);
	tp->run();

	std::atomic<bool> ready = false;

	auto coro = [&]() -> task
	{
		co_await mtx.lock();
		counter++;
		mtx.unlock();
		ready = true;
	};

	taskManager::instance().execute(coro());

	while (!ready)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_FALSE(mtx.locked());
	EXPECT_EQ(counter, 1);
	tp->stop();
}

TEST(CoroMutexTest, CoroExecutionAfterUnlock)
{
	coroMutex mtx;
	mtx.lock();
	int counter = 0;

	auto tp = std::make_shared<threadPool>(1);
	taskManager::instance().init(tp);
	tp->run();

	std::atomic<bool> ready = false;

	auto coro = [&]() -> task
	{
		co_await mtx.lock();
		counter++;
		mtx.unlock();
		ready = true;
	};

	taskManager::instance().execute(coro());

	std::this_thread::sleep_for(std::chrono::milliseconds(100));
	EXPECT_EQ(counter, 0);

	mtx.unlock();

	while (!ready)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_EQ(counter, 1);
	tp->stop();
}

TEST(CoroMutexTest, RaceCondition)
{
	coroMutex mtx;
	int counter = 0;
	constexpr int MAX = 100000;
	constexpr size_t coroCount = 10;

	auto tp = std::make_shared<threadPool>(10);
	taskManager::instance().init(tp);
	tp->run();

	std::atomic<int> completed { 0 };

	auto coro = [&](int max) -> task
	{
		for (int i = 0; i < max; ++i)
		{
			co_await mtx.lock();
			counter++;
			mtx.unlock();
		}
		completed++;
	};

	for (size_t i = 0; i < coroCount; ++i)
	{
		taskManager::instance().execute(coro(MAX));
	}

	while (completed.load() < coroCount)
	{ }

	EXPECT_EQ(counter, MAX * coroCount);
	tp->stop();
}

TEST(CoroMutexTest, RecursiveLock)
{
	coroMutex mtx;
	auto tp = std::make_shared<threadPool>(1);
	taskManager::instance().init(tp);
	tp->run();

	std::atomic<bool> secondLockSuccess { false };
	std::atomic<bool> ready1 { false }, ready2 { false };
	auto recursiveCoro = [&]() -> task
	{
		co_await mtx.lock();
		ready1 = true;
		co_await mtx.lock();
		secondLockSuccess = true;
		mtx.unlock();
		ready2 = true;
	};

	taskManager::instance().execute(recursiveCoro());
	while (!ready1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	EXPECT_FALSE(secondLockSuccess);
	mtx.unlock();

	while (!ready2)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}
	EXPECT_TRUE(secondLockSuccess);

	tp->stop();
}

TEST(CoroMutexTest, LockOrderIsFIFO)
{
	coroMutex mtx;
	auto tp = std::make_shared<threadPool>(1);
	taskManager::instance().init(tp);
	tp->run();

	std::vector<int> executionOrder;
	std::atomic<int> completed { 0 };

	auto coro = [&](int id) -> task
	{
		co_await mtx.lock();
		executionOrder.push_back(id);
		mtx.unlock();
		completed++;
	};

	constexpr int coroCount = 5;
	for (int i = 0; i < coroCount; ++i)
	{
		taskManager::instance().execute(coro(i));
	}

	while (completed.load() < coroCount)
	{ }

	for (int i = 0; i < coroCount; ++i)
	{
		EXPECT_EQ(executionOrder[i], i) << "Lock order should be FIFO!";
	}

	tp->stop();
}
