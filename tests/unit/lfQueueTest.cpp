#include <gtest/gtest.h>

#include "core/lf-queue.h"

#include <thread>
#include <vector>
#include <atomic>
#include <algorithm>


class LFQueueTest : public ::testing::Test
{
protected:
	void SetUp() override
	{
	}

	void TearDown() override
	{
	}

	lfQueue<int> queue;
};

TEST_F(LFQueueTest, IsInitiallyEmpty)
{
	int dummy;
	EXPECT_FALSE(queue.pop(dummy));
}

TEST_F(LFQueueTest, PushPopSingleElement)
{
	queue.push(42);
	int result;
	EXPECT_TRUE(queue.pop(result));
	EXPECT_EQ(result, 42);
	EXPECT_FALSE(queue.pop(result));
}

TEST_F(LFQueueTest, PushPopMultipleElements)
{
	queue.push(1);
	queue.push(2);
	queue.push(3);

	int result;
	EXPECT_TRUE(queue.pop(result));
	EXPECT_EQ(result, 1);
	EXPECT_TRUE(queue.pop(result));
	EXPECT_EQ(result, 2);
	EXPECT_TRUE(queue.pop(result));
	EXPECT_EQ(result, 3);
	EXPECT_FALSE(queue.pop(result));
}

TEST_F(LFQueueTest, MaintainsOrder)
{
	for (int i = 0; i < 100; ++i)
	{
		queue.push(i);
	}

	int result;
	for (int i = 0; i < 100; ++i)
	{
		EXPECT_TRUE(queue.pop(result));
		EXPECT_EQ(result, i);
	}
	EXPECT_FALSE(queue.pop(result));
}

TEST_F(LFQueueTest, ConcurrentPush)
{
	const int num_threads = 4;
	const int items_per_thread = 1000;
	std::vector<std::thread> threads;

	for (int i = 0; i < num_threads; ++i)
	{
		threads.emplace_back(
			[this, i]
			{
				for (int j = 0; j < items_per_thread; ++j)
				{
					queue.push(i * items_per_thread + j);
				}
			});
	}

	for (auto& t : threads)
	{
		t.join();
	}

	std::vector<int> results;
	int val;
	while (queue.pop(val))
	{
		results.push_back(val);
	}

	EXPECT_EQ(results.size(), num_threads * items_per_thread);

	std::sort(results.begin(), results.end());
	for (int i = 0; i < num_threads * items_per_thread; ++i)
	{
		EXPECT_EQ(results[i], i);
	}
}

TEST_F(LFQueueTest, ConcurrentPop)
{
	const int total_items = 1000;
	for (int i = 0; i < total_items; ++i)
	{
		queue.push(i);
	}

	std::atomic<int> popped_count { 0 };
	std::vector<int> popped_items;
	popped_items.resize(total_items);

	const int num_threads = 4;
	std::vector<std::thread> threads;

	for (int i = 0; i < num_threads; ++i)
	{
		threads.emplace_back(
			[this, &popped_count, &popped_items]
			{
				int val;
				while (queue.pop(val))
				{
					popped_items[popped_count++] = val;
				}
			});
	}

	for (auto& t : threads)
	{
		t.join();
	}

	EXPECT_EQ(popped_count, total_items);

	std::sort(popped_items.begin(), popped_items.end());
	for (int i = 0; i < total_items; ++i)
	{
		EXPECT_EQ(popped_items[i], i);
	}
}

TEST_F(LFQueueTest, ConcurrentPushAndPop)
{
	const int num_producers = 2;
	const int num_consumers = 2;
	const int items_per_producer = 500;
	const int total_items = num_producers * items_per_producer;

	std::atomic<int> popped_count { 0 };
	std::vector<int> popped_items;
	popped_items.resize(total_items);

	std::vector<std::thread> producers;
	std::vector<std::thread> consumers;

	for (int i = 0; i < num_producers; ++i)
	{
		producers.emplace_back(
			[this, i]
			{
				for (int j = 0; j < items_per_producer; ++j)
				{
					queue.push(i * items_per_producer + j);
				}
			});
	}

	for (int i = 0; i < num_consumers; ++i)
	{
		consumers.emplace_back(
			[this, &popped_count, &popped_items]
			{
				int val;
				while (popped_count < total_items)
				{
					if (queue.pop(val))
					{
						popped_items[popped_count++] = val;
					}
				}
			});
	}

	for (auto& t : producers)
	{
		t.join();
	}

	for (auto& t : consumers)
	{
		t.join();
	}

	EXPECT_EQ(popped_count, total_items);

	std::sort(popped_items.begin(), popped_items.end());
	for (int i = 0; i < total_items; ++i)
	{
		EXPECT_EQ(popped_items[i], i);
	}
}

TEST_F(LFQueueTest, StressTest)
{
	const int num_threads = 8;
	const int iterations = 1000;
	std::vector<std::thread> threads;

	for (int i = 0; i < num_threads; ++i)
	{
		threads.emplace_back(
			[this, i, iterations]
			{
				for (int j = 0; j < iterations; ++j)
				{
					queue.push(i * iterations + j);
					int val;
					if (queue.pop(val))
					{
						// Just verify we got something
						EXPECT_GE(val, 0);
					}
				}
			});
	}

	for (auto& t : threads)
	{
		t.join();
	}

	int val;
	int count = 0;
	while (queue.pop(val))
	{
		count++;
		EXPECT_GE(val, 0);
	}

	EXPECT_GE(count, 0);
	EXPECT_LE(count, num_threads * iterations);
}