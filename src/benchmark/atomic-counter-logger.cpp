#include "benchmark/atomic-counter-logger.h"

#include <spdlog/spdlog.h>

#include <chrono>
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
	startTime_ = std::chrono::steady_clock::now();
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
		spdlog::error("Failed to increment {} element. Counters number {}", counter_index, counters_.size());
		return;
	}
	counters_[counter_index].fetch_add(1, std::memory_order_relaxed);
	spdlog::trace("Incremented counter: {}, value: {}", counter_index, counters_[counter_index].load());
}

void atomicCounterLogger::decrement(size_t counter_index)
{
	if (counter_index >= counters_.size())
	{
		spdlog::error("Failed to decrement {} element. Counters number {}", counter_index, counters_.size());
		return;
	}
	counters_[counter_index].fetch_sub(1, std::memory_order_relaxed);
	spdlog::trace("Decremented counter: {}, value: {}", counter_index, counters_[counter_index].load());
}

int64_t atomicCounterLogger::get(size_t counter_index) const
{
	if (counter_index >= counters_.size())
	{
		spdlog::error("Failed to get {} element. Counters number {}", counter_index, counters_.size());
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

	auto now = std::chrono::steady_clock::now();
	auto elapsed = now - startTime_;

	std::ofstream out(filename_, std::ios_base::app);
	if (out.is_open())
	{
		// Разбиваем время на компоненты
		auto hours = std::chrono::duration_cast<std::chrono::hours>(elapsed);
		elapsed -= hours;
		auto minutes = std::chrono::duration_cast<std::chrono::minutes>(elapsed);
		elapsed -= minutes;
		auto seconds = std::chrono::duration_cast<std::chrono::seconds>(elapsed);
		elapsed -= seconds;
		auto milliseconds = std::chrono::duration_cast<std::chrono::milliseconds>(elapsed);

		// Форматируем время как "часы:минуты:секунды.миллисекунды"
		out << std::setfill('0') << std::setw(2) << hours.count() << ":" << std::setw(2) << minutes.count() << ":" << std::setw(2) << seconds.count() << "."
				<< std::setw(3) << milliseconds.count() << ",";

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
		spdlog::error("Failed to open file: {}", filename_);
	}
}