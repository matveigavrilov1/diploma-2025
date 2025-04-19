#pragma once

#include <coroutine>
#include <memory>

#include "singleton.h"

#include "task.h"
#include "thread-pool.h"

namespace cs
{
class taskManager : public singleton<taskManager>
{
public:
	taskManager();

	void init(std::shared_ptr<threadPool> tp);

	void execute(std::coroutine_handle<>& taskToExecute);
	void execute(task&& taskToExecute);

private:
	std::shared_ptr<threadPool> tp_ { nullptr };
};
} // namespace cs