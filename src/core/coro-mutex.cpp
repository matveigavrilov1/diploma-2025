#include "coro-mutex.h"

#include "task-manager.h"

cs::coroMutex::awaiter::awaiter(cs::coroMutex& cm, bool locked)
: cm_ { cm }
, locked_(locked)
{ }

bool cs::coroMutex::awaiter::await_ready()
{
	return !locked_;
}

void cs::coroMutex::awaiter::await_suspend(std::coroutine_handle<> handle)
{
	cm_.queue_.enqueue(handle);
}

void cs::coroMutex::awaiter::await_resume() { }

cs::coroMutex::awaiter cs::coroMutex::lock()
{
	bool expected = locked_.load();
	while (!locked_.compare_exchange_weak(expected, true))
	{
		expected = locked_.load();
	}
	return awaiter { *this, expected };
}

void cs::coroMutex::unlock()
{
	std::coroutine_handle<> handle;
	if (queue_.try_dequeue(handle))
	{
		cs::taskManager::instance().execute(handle);
	}
	else
	{
		locked_.store(false);
	}
}