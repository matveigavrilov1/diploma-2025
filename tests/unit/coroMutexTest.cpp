#include <gtest/gtest.h>

#include "core/coro-mutex.h"
#include "core/task-manager.h"

#include <atomic>
#include <thread>
#include <chrono>

using namespace cs;

void waitForCompletion(const std::atomic<bool>& flag, int maxWaitMs = 100)
{
	int waited = 0;
	while (!flag.load() && waited < maxWaitMs)
	{
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
		waited++;
	}
	ASSERT_NE(waited, maxWaitMs) << "Timeout waiting for completion";
}

// Базовые тесты синхронного поведения
TEST(CoroMutexTest, LockSetsLockedFlag)
{
	coroMutex mtx;
	auto awaiter = mtx.lock();
	EXPECT_TRUE(mtx.locked());
}

TEST(CoroMutexTest, UnlockClearsLockedFlag)
{
	coroMutex mtx;
	auto awaiter = mtx.lock();
	mtx.unlock();
	EXPECT_FALSE(mtx.locked());
}

TEST(CoroMutexTest, AwaiterReadyForFirstLock)
{
	coroMutex mtx;
	auto awaiter = mtx.lock();
	EXPECT_TRUE(awaiter.await_ready());
}

TEST(CoroMutexTest, AwaiterNotReadyForSubsequentLocks)
{
	coroMutex mtx;
	auto awaiter1 = mtx.lock();
	auto awaiter2 = mtx.lock();
	EXPECT_FALSE(awaiter2.await_ready());
	mtx.unlock();
}

// Тесты однопоточного асинхронного поведения
class CoroMutexSingleThreadTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		tp = std::make_shared<threadPool>(1);
		taskManager::instance().init(tp);
		tp->run();
	}

	void TearDown() override { tp->stop(); }

	std::shared_ptr<threadPool> tp;
};

TEST_F(CoroMutexSingleThreadTest, SingleCoroLockUnlock)
{
	coroMutex mtx;
	std::atomic<bool> completed = false;

	auto coro = [&]() -> task
	{
		co_await mtx.lock();
		mtx.unlock();
		completed = true;
	};

	taskManager::instance().execute(coro());

	waitForCompletion(completed);
	EXPECT_FALSE(mtx.locked());
}

TEST_F(CoroMutexSingleThreadTest, CoroWaitsForUnlock)
{
	coroMutex mtx;
	mtx.lock();
	std::atomic<bool> completed = false;
	int counter = 0;

	auto coro = [&]() -> task
	{
		co_await mtx.lock();
		counter++;
		mtx.unlock();
		completed = true;
	};

	taskManager::instance().execute(coro());

	// Проверяем, что корутина еще не выполнилась
	std::this_thread::sleep_for(std::chrono::milliseconds(10));
	EXPECT_EQ(counter, 0);

	// Разблокируем и проверяем выполнение
	mtx.unlock();
	waitForCompletion(completed);
	EXPECT_EQ(counter, 1);
}

TEST_F(CoroMutexSingleThreadTest, RecursiveLockBlocks)
{
	coroMutex mtx;
	std::atomic<bool> secondLockObtained = false;
	std::atomic<bool> firstLockCompleted = false;

	auto coro = [&]() -> task
	{
		co_await mtx.lock();
		firstLockCompleted = true;
		co_await mtx.lock(); // Должно заблокироваться
		secondLockObtained = true;
		mtx.unlock();
	};

	taskManager::instance().execute(coro());

	waitForCompletion(firstLockCompleted);
	EXPECT_FALSE(secondLockObtained);

	mtx.unlock(); // Разрешаем вторую блокировку
	waitForCompletion(secondLockObtained);
}

// Тесты многопоточного поведения
class CoroMutexMultiThreadTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
		tp = std::make_shared<threadPool>(10);
		taskManager::instance().init(tp);
		tp->run();
	}

	void TearDown() override { tp->stop(); }

	std::shared_ptr<threadPool> tp;

	void waitForAtomic(std::atomic<int>& value, int expected, int maxWaitMs = 1000)
	{
		int waited = 0;
		while (value.load() < expected && waited < maxWaitMs)
		{
			std::this_thread::sleep_for(std::chrono::milliseconds(1));
			waited++;
		}
		ASSERT_NE(waited, maxWaitMs);
	}
};

TEST_F(CoroMutexMultiThreadTest, NoRaceCondition)
{
	coroMutex mtx;
	std::atomic<int> counter = 0;
	constexpr int iterations = 10000;
	constexpr int coroCount = 10;
	std::atomic<int> completed = 0;

	auto coro = [&]() -> task
	{
		for (int i = 0; i < iterations; ++i)
		{
			co_await mtx.lock();
			counter++;
			mtx.unlock();
		}
		completed++;
	};

	for (int i = 0; i < coroCount; ++i)
	{
		taskManager::instance().execute(coro());
	}

	waitForAtomic(completed, coroCount);
	EXPECT_EQ(counter, iterations * coroCount);
}

TEST_F(CoroMutexMultiThreadTest, FIFOOrdering)
{
	coroMutex mtx;
	std::vector<int> lockOrder;
	std::vector<int> unlockOrder;
	std::mutex vecMutex; // Для защиты векторов от одновременного доступа
	std::atomic<int> completed = 0;
	constexpr int coroCount = 5;

	auto coro = [&](int id) -> task
	{
		{
			std::lock_guard<std::mutex> lock(vecMutex);
			lockOrder.push_back(id);
			co_await mtx.lock();
		}

		{
			std::lock_guard<std::mutex> lock(vecMutex);
			unlockOrder.push_back(id);
		}

		mtx.unlock();
		completed++;
	};

	for (int i = 0; i < coroCount; ++i)
	{
		taskManager::instance().execute(coro(i));
		std::this_thread::sleep_for(std::chrono::milliseconds(1));
	}

	waitForAtomic(completed, coroCount, 1000);
	for (int i = 0; i < coroCount; ++i)
	{
		EXPECT_EQ(lockOrder[i], unlockOrder[i]) << "Execution should be FIFO";
	}
}