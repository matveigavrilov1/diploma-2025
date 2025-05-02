#include "benchmark/counter/atomic-multiple-counter.h"

#include <spdlog/spdlog.h>

#include <ctime>
#include <numeric>

using namespace cs;

atomicMultipleCounter::atomicMultipleCounter(size_t counterCount)
: counters_(counterCount)
{
	for (auto& counter : counters_)
	{
		counter.store(0, std::memory_order_relaxed);
	}
}

void atomicMultipleCounter::increment(size_t counter_index)
{
	if (counter_index >= counters_.size())
	{
		spdlog::error("Failed to increment {} element. Counters number {}", counter_index, counters_.size());
		return;
	}
	counters_[counter_index].fetch_add(1, std::memory_order_relaxed);
	spdlog::trace("Incremented counter: {}, value: {}", counter_index, counters_[counter_index].load());
}

void atomicMultipleCounter::decrement(size_t counter_index)
{
	if (counter_index >= counters_.size())
	{
		spdlog::error("Failed to decrement {} element. Counters number {}", counter_index, counters_.size());
		return;
	}
	counters_[counter_index].fetch_sub(1, std::memory_order_relaxed);
	spdlog::trace("Decremented counter: {}, value: {}", counter_index, counters_[counter_index].load());
}

int64_t atomicMultipleCounter::get(size_t counter_index) const
{
	if (counter_index >= counters_.size())
	{
		spdlog::error("Failed to get {} element. Counters number {}", counter_index, counters_.size());
		return 0;
	}
	return counters_[counter_index].load(std::memory_order_relaxed);
}

int64_t atomicMultipleCounter::get_total() const
{
	return std::accumulate(counters_.begin(), counters_.end(), int64_t { 0 },
		[](int64_t sum, const std::atomic<int64_t>& counter) { return sum + counter.load(std::memory_order_relaxed); });
}

size_t atomicMultipleCounter::size() const
{
	return counters_.size();
}