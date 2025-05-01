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

	size_t maxWaitingTime = 100;
	size_t waitingTime = 0;
	while (!ready && waitingTime < maxWaitingTime)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		waitingTime++;
	}
	ASSERT_NE(waitingTime, maxWaitingTime);

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

	size_t maxWaitingTime = 100;
	size_t waitingTime = 0;
	while (!ready && waitingTime < maxWaitingTime)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		waitingTime++;
	}
	ASSERT_NE(waitingTime, maxWaitingTime);

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

	size_t maxWaitingTime = 10000;
	size_t waitingTime = 0;
	while (completed.load() < coroCount && waitingTime < maxWaitingTime)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds { true });
		waitingTime++;
	}
	ASSERT_NE(waitingTime, maxWaitingTime);

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

	size_t maxWaitingTime1 = 100;
	size_t waitingTime1 = 0;
	while (!ready1 && waitingTime1 < maxWaitingTime1)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		waitingTime1++;
	}
	ASSERT_NE(waitingTime1, maxWaitingTime1);

	EXPECT_FALSE(secondLockSuccess);
	mtx.unlock();

	size_t maxWaitingTime2 = 100;
	size_t waitingTime2 = 0;
	while (!ready2 && waitingTime2 < maxWaitingTime2)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		waitingTime2++;
	}
	ASSERT_NE(waitingTime2, maxWaitingTime2);

	EXPECT_TRUE(secondLockSuccess);

	tp->stop();
}

TEST(CoroMutexTest, ExecutionOrdering)
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

	size_t maxWaitingTime = 100;
	size_t waitingTime = 0;
	while (completed.load() < coroCount && waitingTime < maxWaitingTime)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		waitingTime++;
	}
	ASSERT_NE(waitingTime, maxWaitingTime);

	for (int i = 0; i < coroCount; ++i)
	{
		EXPECT_EQ(executionOrder[i], i) << "Lock order should be FIFO!";
	}

	tp->stop();
}
