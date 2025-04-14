#pragma once

#include <atomic>

template<typename T>
struct node
{
	T data;
	std::atomic<node*> next;

	node(const T& data)
	: data(data)
	, next(nullptr)
	{ }
};

template<typename T>
class lfQueue
{
	std::atomic<node<T>*> head;
	std::atomic<node<T>*> tail;

public:
	lfQueue()
	{
		node<T>* dummy = new node<T>(T {});
		head.store(dummy, std::memory_order_relaxed);
		tail.store(dummy, std::memory_order_relaxed);
	}

	~lfQueue()
	{
		T val;
		while (pop(val))
			;
		delete head.load(std::memory_order_relaxed);
	}

	void push(const T& data)
	{
		node<T>* new_node = new node<T>(data);

		node<T>* current_tail = nullptr;
		node<T>* next = nullptr;

		while (true)
		{
			current_tail = tail.load(std::memory_order_acquire);
			next = current_tail->next.load(std::memory_order_acquire);

			if (current_tail == tail.load(std::memory_order_acquire))
			{
				if (next == nullptr)
				{
					if (current_tail->next.compare_exchange_strong(next, new_node, std::memory_order_release, std::memory_order_relaxed))
					{
						break;
					}
				}
				else
				{
					tail.compare_exchange_strong(current_tail, next, std::memory_order_release, std::memory_order_relaxed);
				}
			}
		}

		tail.compare_exchange_strong(current_tail, new_node, std::memory_order_release, std::memory_order_relaxed);
	}

	bool pop(T& result)
	{
		node<T>* current_head = nullptr;
		node<T>* current_tail = nullptr;
		node<T>* next = nullptr;

		while (true)
		{
			current_head = head.load(std::memory_order_acquire);
			current_tail = tail.load(std::memory_order_acquire);
			next = current_head->next.load(std::memory_order_acquire);

			if (current_head == head.load(std::memory_order_acquire))
			{
				if (current_head == current_tail)
				{
					if (next == nullptr)
					{
						return false; // Queue is empty
					}
					tail.compare_exchange_weak(current_tail, next, std::memory_order_release, std::memory_order_relaxed);
				}
				else
				{
					result = next->data;
					if (head.compare_exchange_weak(current_head, next, std::memory_order_release, std::memory_order_relaxed))
					{
						delete current_head;
						return true;
					}
				}
			}
		}
	}
};