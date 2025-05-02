#pragma once

#include <atomic>
#include <vector>

namespace cs
{

class atomicMultipleCounter
{
public:
	atomicMultipleCounter(size_t counterCount = 1);

	atomicMultipleCounter(const atomicMultipleCounter&) = delete;
	atomicMultipleCounter& operator= (const atomicMultipleCounter&) = delete;

	void increment(size_t counterIndex = 0);
	void decrement(size_t counterIndex = 0);

	int64_t get(size_t counterIndex = 0) const;
	int64_t get_total() const;

	size_t size() const;

private:
	std::vector<std::atomic<int64_t>> counters_;
};
} // namespace cs