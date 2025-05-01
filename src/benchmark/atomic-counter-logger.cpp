

#include "benchmark/atomic-counter-logger.h"

#include <fstream>
#include <iostream>
#include <ctime>
#include <iomanip>

using namespace cs;

AtomicCounterLogger::AtomicCounterLogger(const std::string& filename, std::chrono::milliseconds interval)
: counter_(0)
, filename_(filename)
, interval_(interval)
, running_(false)
{ }

AtomicCounterLogger::~AtomicCounterLogger()
{
	stop();
}

void AtomicCounterLogger::start()
{
	if (running_)
		return;

	running_ = true;
	worker_thread_ = std::thread(&AtomicCounterLogger::worker, this);
}

void AtomicCounterLogger::stop()
{
	running_ = false;
	if (worker_thread_.joinable())
	{
		worker_thread_.join();
	}
	dump_counter();
}

void AtomicCounterLogger::increment()
{
	counter_.fetch_add(1, std::memory_order_relaxed);
}

void AtomicCounterLogger::decrement()
{
	counter_.fetch_sub(1, std::memory_order_relaxed);
}

// Префиксный инкремент
AtomicCounterLogger& AtomicCounterLogger::operator++ ()
{
	counter_.fetch_add(1, std::memory_order_relaxed);
	return *this;
}

// Префиксный декремент
AtomicCounterLogger& AtomicCounterLogger::operator-- ()
{
	counter_.fetch_sub(1, std::memory_order_relaxed);
	return *this;
}


int64_t AtomicCounterLogger::get() const
{
	return counter_.load(std::memory_order_relaxed);
}

void AtomicCounterLogger::worker()
{
	while (running_)
	{
		std::this_thread::sleep_for(interval_);
		dump_counter();
	}
}

void AtomicCounterLogger::dump_counter()
{
	std::lock_guard<std::mutex> lock(file_mutex_);

	auto now = std::chrono::system_clock::now();
	std::time_t now_time = std::chrono::system_clock::to_time_t(now);

	std::ofstream out(filename_, std::ios_base::app);
	if (out.is_open())
	{
		out << std::put_time(std::localtime(&now_time), "%Y-%m-%d %H:%M:%S") << "," << get() << "\n";
	}
	else
	{
		std::cerr << "Failed to open file: " << filename_ << std::endl;
	}
}