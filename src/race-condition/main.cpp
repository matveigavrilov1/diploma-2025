#include <coroutine>
#include <iostream>

#include "core/task-manager.h"
#include "core/thread-pool.h"

#include "consts.h"

cs::task producer(int& x, int id)
{
	for (size_t i = 0; i < MAX; ++i)
	{
        co_await std::suspend_never{};
		++x;
	}
	std::cout << "producer " << id << " finished: " << x << std::endl;
}

int main()
{
	int x = 0;
	auto tp = std::make_shared<cs::threadPool>(THREAD_COUNT);
	cs::taskManager::instance().init(tp);

	tp->start();
	for (size_t i = 0; i < PROD_COUNT; ++i)
	{
		cs::taskManager::instance().execute(producer(x, i));
	}

	while (tp->running())
	{ }

	return 0;
}