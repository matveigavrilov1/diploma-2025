#pragma once

#include <atomic>
#include <coroutine>
#include <memory>

#include "singleton.h"

#include "task.h"
#include "thread-pool.h"
#include "ts-queue.h"

namespace cs
{
class taskManager : public singleton<taskManager>
{
public:
	taskManager();

	void init(std::shared_ptr<threadPool> tp);
	void run();
	void stop();

	void execute(std::coroutine_handle<>& taskToExecute);
	void execute(task&& taskToExecute);


private:
	std::atomic<bool> running_ { false };
	tsQueue<task> tasksToResume_;
	std::shared_ptr<threadPool> tp_ { nullptr };
};
} // namespace cs