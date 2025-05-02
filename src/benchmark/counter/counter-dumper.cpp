#include "benchmark/counter/counter-dumper.h"

#include <chrono>
#include <fstream>

#include <spdlog/spdlog.h>

using namespace cs;

counterDumper::counterDumper(atomicMultipleCounter& counter, const std::string& filename, const std::chrono::milliseconds& interval)
: counter_ { counter }
, filename_ { filename }
, interval_ { interval }
{ }

counterDumper::~counterDumper()
{
	stop();
}

void counterDumper::start()
{
	if (running_)
		return;

	running_ = true;
	startTime_ = std::chrono::steady_clock::now();
	worker_ = std::thread(&counterDumper::worker, this);
}

void counterDumper::stop()
{
	running_ = false;
	if (worker_.joinable())
	{
		worker_.join();
	}
	dump();
}

void counterDumper::worker()
{
	while (running_)
	{
		std::this_thread::sleep_for(interval_);
		dump();
	}
}

void counterDumper::dump()
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
		for (size_t i = 0; i < counter_.size(); ++i)
		{
			if (i != 0)
				out << ",";
			out << counter_.get(i);
		}

		// И общую сумму
		out << "," << counter_.get_total() << "\n";
	}
	else
	{
		spdlog::error("Failed to open file: {}", filename_);
	}
}