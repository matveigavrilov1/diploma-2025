#include <iostream>

#include "benchmark/atomic-counter-logger.h"

int main()
{
	cs::AtomicCounterLogger counter("counter_log.csv", std::chrono::seconds(1));
	counter.start();

	for (int i = 0; i < 20; ++i)
	{
		++counter;
		std::this_thread::sleep_for(std::chrono::seconds(1));
	}

	counter.stop();
	return 0;
}