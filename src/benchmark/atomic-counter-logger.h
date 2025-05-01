#pragma once

#include <atomic>
#include <chrono>
#include <string>
#include <thread>
#include <mutex>

namespace cs
{

class AtomicCounterLogger
{
public:
	AtomicCounterLogger(const std::string& filename, std::chrono::milliseconds interval);
	~AtomicCounterLogger();

	AtomicCounterLogger(const AtomicCounterLogger&) = delete;
	AtomicCounterLogger& operator= (const AtomicCounterLogger&) = delete;

	void start();
	void stop();
	void increment();
	void decrement();

	AtomicCounterLogger& operator++ ();
	AtomicCounterLogger& operator-- ();

	int64_t get() const;

private:
	void worker();
	void dump_counter();

	std::atomic<int64_t> counter_;
	std::string filename_;
	std::chrono::milliseconds interval_;
	std::thread worker_thread_;
	std::atomic<bool> running_;
	std::mutex file_mutex_;
};
} // namespace cs