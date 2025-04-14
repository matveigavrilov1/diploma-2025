#include "task.h"

#include "task-manager.h"
#include <coroutine>
#include <utility>

std::suspend_always cs::task::promise_type::initial_suspend() noexcept
{
	return std::suspend_always {};
}

std::suspend_always cs::task::promise_type::final_suspend() noexcept
{
	return std::suspend_always {};
}

void cs::task::promise_type::unhandled_exception() { }

cs::task cs::task::promise_type::get_return_object()
{
	return coro_handle::from_promise(*this);
}

void cs::task::promise_type::return_void() { }

template <>
std::suspend_always cs::task::promise_type::await_transform<std::suspend_always>(std::suspend_always)
{
	taskManager::instance().execute(coro_handle::from_promise(*this));
	return std::suspend_always {};
}

cs::task::task(coro_handle handle)
: handle_(handle)
{ }

cs::task::task(task& other) noexcept
: handle_(other.handle_)
{ }

cs::task::task(task&& other) noexcept
: handle_(std::exchange(other.handle_, nullptr))
{ }

cs::task::~task() { }

cs::task& cs::task::operator= (cs::task& other) noexcept
{
	handle_ = other.handle_;
	return *this;
}

cs::task& cs::task::operator= (cs::task&& other) noexcept
{
	handle_ = std::exchange(other.handle_, nullptr);
	return *this;
}

bool cs::task::resume()
{
	if (!handle_)
		return false;
	if (handle_.done())
		return false;
	handle_.resume();
	return true;
}

bool cs::task::done()
{
	if (handle_)
		return handle_.done();
	return true;
}

cs::task::coro_handle& cs::task::handle()
{
	return handle_;
}