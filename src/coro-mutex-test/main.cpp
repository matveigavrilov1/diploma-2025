#include <coroutine>
#include <iostream>

#include "task-manager.h"
#include "thread-pool.h"
#include "coro-mutex.h"

#include "src/consts.h"

cs::coroMutex mtx;

cs::task producer(int& x, int id)
{
	for (size_t i = 0; i < MAX; ++i)
	{
		co_await mtx.lock();
		++x;
		mtx.unlock();
	}
	std::cout << "producer " << id << " finished: " << x << std::endl;
}

int main()
{
	int x = 0;
	auto tp = std::make_shared<cs::threadPool>(THREAD_COUNT);
	cs::taskManager::instance().init(tp);

	tp->run();
	for (size_t i = 0; i < PROD_COUNT; ++i)
	{
		cs::taskManager::instance().execute(producer(x, i));
	}

	while (tp->running())
	{ }

	return 0;
}