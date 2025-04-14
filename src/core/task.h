#pragma once

#include <coroutine>

namespace cs
{

class task
{
public:
	struct promise_type
	{
		using coro_handle = std::coroutine_handle<promise_type>;

		std::suspend_always initial_suspend() noexcept;
		std::suspend_always final_suspend() noexcept;
		void unhandled_exception();
		task get_return_object();
		void return_void();

		template<typename T>
		T await_transform(T value)
		{
			return value;
		}
	};

	using coro_handle = std::coroutine_handle<promise_type>;

	task(coro_handle handle);
	task(task& other) noexcept;
	task(task&& other) noexcept;
	~task();

	task& operator= (task& other) noexcept;
	task& operator= (task&& other) noexcept;

	bool resume();
	bool done();

	coro_handle& handle();

private:
	coro_handle handle_;
};

} // namespace cs