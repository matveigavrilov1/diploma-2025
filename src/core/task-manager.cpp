#include "task-manager.h"

namespace cs
{
taskManager::taskManager() { }

void taskManager::init(std::shared_ptr<threadPool> tp)
{
	tp_ = tp;
}

void taskManager::run()
{
	if (running_)
		return;
	running_ = true;
	while (running_)
	{ }
}

void taskManager::stop()
{
	running_ = false;
}

void taskManager::execute(std::coroutine_handle<>& taskToExecute)
{
	auto lambdaTask = [handle = taskToExecute]() mutable
	{
		handle.resume();
	};
	if (!taskToExecute.done() && tp_)
		tp_->pushTask(std::move(lambdaTask));
}

void taskManager::execute(task&& taskToExecute)
{
    std::coroutine_handle<> handle = taskToExecute.handle();
	execute(handle);
}
} // namespace cs