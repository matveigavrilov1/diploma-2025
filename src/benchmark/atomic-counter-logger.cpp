#include "benchmark/atomic-counter-logger.h"

#include <fstream>
#include <numeric>
#include <iostream>
#include <ctime>
#include <iomanip>

using namespace cs;

atomicCounterLogger::atomicCounterLogger(const std::string& filename, std::chrono::milliseconds interval, size_t counter_count)
: counters_(counter_count)
, filename_(filename)
, interval_(interval)
, running_(false)
{
	for (auto& counter : counters_)
	{
		counter.store(0, std::memory_order_relaxed);
	}
}

atomicCounterLogger::~atomicCounterLogger()
{
	stop();
}

void atomicCounterLogger::start()
{
	if (running_)
		return;

	running_ = true;
	worker_ = std::thread(&atomicCounterLogger::worker, this);
}

void atomicCounterLogger::stop()
{
	running_ = false;
	if (worker_.joinable())
	{
		worker_.join();
	}
	dump();
}

void atomicCounterLogger::increment(size_t counter_index)
{
	if (counter_index >= counters_.size())
	{
		return;
	}
	counters_[counter_index].fetch_add(1, std::memory_order_relaxed);
}

void atomicCounterLogger::decrement(size_t counter_index)
{
	if (counter_index >= counters_.size())
	{
		return;
	}
	counters_[counter_index].fetch_sub(1, std::memory_order_relaxed);
}

int64_t atomicCounterLogger::get(size_t counter_index) const
{
	if (counter_index >= counters_.size())
	{
		return 0;
	}
	return counters_[counter_index].load(std::memory_order_relaxed);
}

int64_t atomicCounterLogger::get_total() const
{
	return std::accumulate(counters_.begin(), counters_.end(), int64_t { 0 },
		[](int64_t sum, const std::atomic<int64_t>& counter) { return sum + counter.load(std::memory_order_relaxed); });
}

void atomicCounterLogger::worker()
{
	while (running_)
	{
		std::this_thread::sleep_for(interval_);
		dump();
	}
}

void atomicCounterLogger::dump()
{
	std::lock_guard<std::mutex> lock(mtx_);

	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	std::ofstream out(filename_, std::ios_base::app);
	if (out.is_open())
	{
		out << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << ",";

		// Записываем каждый счетчик
		for (size_t i = 0; i < counters_.size(); ++i)
		{
			if (i != 0)
				out << ",";
			out << get(i);
		}

		// И общую сумму
		out << "," << get_total() << "\n";
	}
	else
	{
		std::cerr << "Failed to open file: " << filename_ << std::endl;
	}
}