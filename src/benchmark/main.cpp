#include <chrono>
#include <csignal>
#include <optional>
#include <string>

#include "benchmark/atomic-counter-logger.h"
#include "core/thread-pool.h"
#include "optionsManager/options-manager.h"
#include "optionsManager/register-option.h"

std::atomic<bool> running { true };
std::optional<cs::threadPool> tp;
std::optional<cs::atomicCounterLogger> counter;

void signalHandler(int signal)
{
	if (signal == SIGTERM)
	{
		running = false;
		if (tp)
		{
			tp->stop();
		}
		if (counter)
		{
			counter->stop();
		}
	}
}

REGISTER_OPTION("help", 'h', helpOption, bool, false);
REGISTER_OPTION("threads-number", 'n', threadsNumberOption, size_t, 10);
REGISTER_OPTION("coro-number", 'c', coroNumberOption, size_t, 5);
REGISTER_OPTION("shared-number", 's', sharedNumberOption, size_t, 1);
REGISTER_OPTION("target", 't', targetOption, std::string, "cm");
REGISTER_OPTION("dump-period", 'd', dumpPeriodOption, size_t, 1000);
REGISTER_OPTION("working-time", 'w', workingTimeOption, size_t, 20);

void setUpOptions(cs::optionsParser& parser)
{
	parser.addOption(helpOptionName, helpOptionShortName, "Show this help message");
	parser.addOption(threadsNumberOptionName, threadsNumberOptionShortName, "Thread pool for coro execution size", true);
	parser.addOption(coroNumberOptionName, coroNumberOptionShortName, "Coroutines number", true);
	parser.addOption(sharedNumberOptionName, sharedNumberOptionShortName, "Number of shared objects", true);
	parser.addOption(targetOptionName, targetOptionShortName, "Target sync prim (m - std::mutex, cm - coroMutex)", true);
	parser.addOption(dumpPeriodOptionName, dumpPeriodOptionShortName, "Period to dump atomic counter, as ms", true);
	parser.addOption(workingTimeOptionName, workingTimeOptionShortName, "Time to work, as seconds (inf - infinite loop)", true);
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
}

std::string getLogFileName()
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
                   << "worktime_" << workingTimeOption
                   << ".csv";
	// clang-format on

	return filenameStream.str();
}

int main(int argc, char* argv[])
{
	// signal handling
	std::signal(SIGTERM, signalHandler);

	// options parsing
	cs::optionsParser parser;
	setUpOptions(parser);
	parser.parse(argc, argv);
	cs::optionsManager options(parser);
	serializeOptions(options);

	if (helpOption)
	{
		parser.printHelp();
		return 0;
	}

	counter.emplace(getLogFileName(), std::chrono::milliseconds(dumpPeriodOption), sharedNumberOption);
	tp.emplace(threadsNumberOption);

	counter->start();
    tp->start();

	return 0;
}