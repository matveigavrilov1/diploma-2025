#pragma once

#include <atomic>
#include <mutex>
#include <string>
#include <thread>

#include "benchmark/counter/atomic-multiple-counter.h"

namespace cs
{
class counterDumper
{
public:
	counterDumper(atomicMultipleCounter& counter, const std::string& filename, const std::chrono::milliseconds& interval);
	~counterDumper();

	counterDumper(const counterDumper& other) = delete;
	counterDumper& operator= (const counterDumper& other) = delete;

	void start();
	void stop();

private:
	void worker();
	void dump();

private:
	atomicMultipleCounter& counter_;

	std::string filename_;
	std::chrono::milliseconds interval_;
	std::thread worker_;
	std::atomic<bool> running_ {false};
	std::mutex mtx_;
	std::chrono::time_point<std::chrono::steady_clock> startTime_;
};

} // namespace cs