#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <mutex>
#include <vector>

namespace cs
{

class atomicCounterLogger
{
public:
	atomicCounterLogger(const std::string& filename, std::chrono::milliseconds interval, size_t counter_count = 1);
	~atomicCounterLogger();

	atomicCounterLogger(const atomicCounterLogger&) = delete;
	atomicCounterLogger& operator= (const atomicCounterLogger&) = delete;

	void start();
	void stop();
	void increment(size_t counter_index = 0);
	void decrement(size_t counter_index = 0);

	int64_t get(size_t counter_index = 0) const;
	int64_t get_total() const;

private:
	void worker();
	void dump();

	std::vector<std::atomic<int64_t>> counters_;
	std::string filename_;
	std::chrono::milliseconds interval_;
	std::thread worker_;
	std::atomic<bool> running_;
	std::mutex mtx_;
	std::chrono::time_point<std::chrono::steady_clock> startTime_;
};
} // namespace cs