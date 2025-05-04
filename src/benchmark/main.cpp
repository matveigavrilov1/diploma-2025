#include <chrono>
#include <csignal>
#include <memory>
#include <optional>
#include <string>
#include <sys/resource.h>
#include <thread>
#include <iostream>
#include <fstream>

#include <spdlog/spdlog.h>
#include <spdlog/sinks/basic_file_sink.h>
#include <spdlog/sinks/stdout_color_sinks.h>

#include "benchmark/counter/atomic-multiple-counter.h"
#include "benchmark/counter/counter-dumper.h"
#include "benchmark/coro.h"

#include "core/coro-mutex.h"
#include "core/task-manager.h"
#include "core/thread-pool.h"

#include "optionsManager/options-manager.h"
#include "optionsManager/register-option.h"

std::atomic<bool> running { true };
std::shared_ptr<cs::threadPool> tp;
std::optional<cs::atomicMultipleCounter> counter;
std::optional<cs::counterDumper> counterDumper;

void signalHandler(int signal);

REGISTER_OPTION("help", 'h', helpOption, bool, false);
REGISTER_OPTION("threads-number", 'n', threadsNumberOption, size_t, 10);
REGISTER_OPTION("coro-number", 'c', coroNumberOption, size_t, 5);
REGISTER_OPTION("shared-number", 's', sharedNumberOption, size_t, 1);
REGISTER_OPTION("target", 't', targetOption, std::string, "cm");
REGISTER_OPTION("dump-period", 'd', dumpPeriodOption, size_t, 1000);
REGISTER_OPTION("working-time", 'w', workingTimeOption, size_t, 20);
REGISTER_OPTION("output-dir", 'o', outputDirOption, std::string, ".");


void setUpOptions(cs::optionsParser& parser);
void serializeOptions(cs::optionsManager& options);

std::string getLogFilesBase();
std::string getCounterLogFilePath();
std::string getLogFilePath();
std::string getUsageFilePath();

std::string logFilesBase;

void initLogger();

void dumpUsage(rusage& startUsage, rusage& endUsage, std::chrono::time_point<std::chrono::high_resolution_clock> start,
	std::chrono::time_point<std::chrono::high_resolution_clock> end);

int main(int argc, char* argv[])
{
	struct rusage startUsage, endUsage;
	getrusage(RUSAGE_SELF, &startUsage);
	auto start = std::chrono::high_resolution_clock::now();

	std::signal(SIGTERM, signalHandler);

	cs::optionsParser parser;
	setUpOptions(parser);
	parser.parse(argc, argv);
	cs::optionsManager options(parser);
	serializeOptions(options);

	logFilesBase = getLogFilesBase();
	initLogger();

	spdlog::info("Parsed command line options:");
	spdlog::info("  help: {}", helpOption);
	spdlog::info("  threads-number (-n): {}", threadsNumberOption);
	spdlog::info("  coro-number (-c): {}", coroNumberOption);
	spdlog::info("  shared-number (-s): {}", sharedNumberOption);
	spdlog::info("  target (-t): {}", targetOption);
	spdlog::info("  dump-period (-d): {} ms", dumpPeriodOption);
	spdlog::info("  working-time (-w): {} seconds", workingTimeOption);

	if (helpOption)
	{
		spdlog::info("Showing help message");
		parser.printHelp();
		return 0;
	}

	// initialization
	spdlog::info("Initializing components");
	try
	{
		counter.emplace(sharedNumberOption);
		spdlog::debug("Counter initialized with shared objects number: {}", sharedNumberOption);

		counterDumper.emplace(*counter, getCounterLogFilePath(), std::chrono::milliseconds(dumpPeriodOption));
		spdlog::debug("Counter initialized with dump period: {} ms, and filepath: {}", dumpPeriodOption, getCounterLogFilePath());

		tp = std::make_shared<cs::threadPool>(threadsNumberOption);
		spdlog::debug("Thread pool initialized with {} threads", threadsNumberOption);

		cs::taskManager::instance().init(tp);
		spdlog::debug("Task manager initialized");
	}
	catch (const std::exception& e)
	{
		spdlog::error("Initialization failed: {}", e.what());
		return 1;
	}

	std::vector<std::mutex> mtxVec(sharedNumberOption);
	std::vector<cs::coroMutex> coroMtxVec(sharedNumberOption);
	// workers start
	counterDumper->start();

	spdlog::info("Starting {} threads", threadsNumberOption);
	tp->start();

	// coroutines start
	spdlog::info("Starting {} coroutines", coroNumberOption);
	for (size_t i = 0; i < coroNumberOption; ++i)
	{
		try
		{
			size_t idx = i % sharedNumberOption;
			if (targetOption == "m")
			{
				cs::taskManager::instance().execute(coroutine(*counter, i, running, mtxVec[idx], idx));
				spdlog::debug("Started coroutine {} with std::mutex. counter idx: {}", i, idx);
			}
			else
			{
				cs::taskManager::instance().execute(coroutine(*counter, i, running, coroMtxVec[idx], idx));
				spdlog::debug("Started coroutine {} with coroMutex. counter idx: {}", i, idx);
			}
		}
		catch (const std::exception& e)
		{
			spdlog::error("Failed to start coroutine {}: {}", i, e.what());
		}
	}


	// waiting
	spdlog::info("Running for {} seconds", workingTimeOption);
	std::this_thread::sleep_for(std::chrono::seconds(workingTimeOption));

	// finish
	spdlog::info("Shutting down");
	running = false;
	tp->stop();
	counterDumper->stop();

	getrusage(RUSAGE_SELF, &endUsage);
	auto end = std::chrono::high_resolution_clock::now();

	dumpUsage(startUsage, endUsage, start, end);

	spdlog::info("Benchmark finished successfully");
	spdlog::shutdown();
	return 0;
}

void signalHandler(int signal)
{
	if (signal == SIGTERM)
	{
		spdlog::info("Got SIGTERM");

		running = false;
		if (tp)
		{
			tp->stop();
		}
		if (counter)
		{
			counterDumper->stop();
		}
		std::exit(0);
	}
}

void setUpOptions(cs::optionsParser& parser)
{
	parser.addOption(helpOptionName, helpOptionShortName, "Show this help message");
	parser.addOption(threadsNumberOptionName, threadsNumberOptionShortName, "Thread pool for coro execution size", true);
	parser.addOption(coroNumberOptionName, coroNumberOptionShortName, "Coroutines number", true);
	parser.addOption(sharedNumberOptionName, sharedNumberOptionShortName, "Number of shared objects", true);
	parser.addOption(targetOptionName, targetOptionShortName, "Target sync prim (m - std::mutex, cm - coroMutex)", true);
	parser.addOption(dumpPeriodOptionName, dumpPeriodOptionShortName, "Period to dump atomic counter, as ms", true);
	parser.addOption(workingTimeOptionName, workingTimeOptionShortName, "Time to work, as seconds (inf - infinite loop)", true);
	parser.addOption(outputDirOptionName, outputDirOptionShortName, "Time to work, as seconds (inf - infinite loop)", true);
}

void serializeOptions(cs::optionsManager& options)
{
	helpOption = options.getBool(helpOptionName, helpOption);
	threadsNumberOption = options.getUInt64(threadsNumberOptionName, threadsNumberOption);
	coroNumberOption = options.getUInt64(coroNumberOptionName, coroNumberOption);
	sharedNumberOption = options.getUInt64(sharedNumberOptionName, sharedNumberOption);
	targetOption = options.getString(targetOptionName, targetOption);
	dumpPeriodOption = options.getUInt64(dumpPeriodOptionName, dumpPeriodOption);
	workingTimeOption = options.getUInt64(workingTimeOptionName, workingTimeOption);
	outputDirOption = options.getString(outputDirOptionName, outputDirOption);
}

std::string getLogFilesBase()
{
	auto now = std::chrono::system_clock::now();
	auto in_time_t = std::chrono::system_clock::to_time_t(now);
	std::tm tm = *std::localtime(&in_time_t);

	std::ostringstream timeStream;
	timeStream << std::put_time(&tm, "%Y-%m-%d_%H-%M-%S");

	// clang-format off
    std::ostringstream filenameStream;
    filenameStream << timeStream.str() << "_"
                   << "threads_" << threadsNumberOption << "_"
                   << "coro_" << coroNumberOption << "_"
                   << "shared_" << sharedNumberOption << "_"
                   << "target_" << targetOption << "_"
                   << "dump_" << dumpPeriodOption << "_"
                   << "worktime_" << workingTimeOption;
	// clang-format on

	return filenameStream.str();
}

std::string getCounterLogFilePath()
{
	return outputDirOption + "/" + logFilesBase + ".csv";
}

std::string getLogFilePath()
{
	return outputDirOption + "/" + logFilesBase + ".log";
}

std::string getUsageFilePath()
{
	return outputDirOption + "/" + logFilesBase + ".usage";
}

void initLogger()
{
	try
	{
		auto console_sink = std::make_shared<spdlog::sinks::stdout_color_sink_mt>();
		auto file_sink = std::make_shared<spdlog::sinks::basic_file_sink_mt>(getLogFilePath(), true);

		std::vector<spdlog::sink_ptr> sinks { console_sink, file_sink };
		auto logger = std::make_shared<spdlog::logger>("main", begin(sinks), end(sinks));

		logger->set_level(spdlog::level::debug);
		spdlog::set_default_logger(logger);
	}
	catch (const spdlog::spdlog_ex& ex)
	{
		std::cerr << "Log initialization failed: " << ex.what() << std::endl;
		std::exit(1);
	}
}

void dumpUsage(rusage& startUsage, rusage& endUsage, std::chrono::time_point<std::chrono::high_resolution_clock> start,
	std::chrono::time_point<std::chrono::high_resolution_clock> end)
{
	int64_t userTime = (endUsage.ru_utime.tv_sec - startUsage.ru_utime.tv_sec) * INT64_C(1000000);
	userTime += (endUsage.ru_utime.tv_usec - startUsage.ru_utime.tv_usec);

	int64_t systemTime = (endUsage.ru_stime.tv_sec - startUsage.ru_stime.tv_sec) * INT64_C(1000000);
	systemTime += (endUsage.ru_stime.tv_usec - startUsage.ru_stime.tv_usec);

	auto duration = std::chrono::duration_cast<std::chrono::microseconds>(end - start);
	int64_t wallTime = duration.count();

	spdlog::info("Wall Time: {} μs", wallTime);
	spdlog::info("User Time: {} μs", userTime);
	spdlog::info("System Time: {} μs", systemTime);

	std::string filename = getUsageFilePath();
	std::ofstream outfile(filename, std::ios::app);
	if (outfile.is_open())
	{
		outfile << "=== Resource Usage ===" << "\n";
		outfile << "Wall Time (μs): " << wallTime << "\n";
		outfile << "User Time (μs): " << userTime << "\n";
		outfile << "System Time (μs): " << systemTime << "\n";
		outfile << "======================" << "\n\n";
		outfile.close();
	}
	else
	{
		spdlog::error("Failed to open file {} for writing!", filename);
	}
}