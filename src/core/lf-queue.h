#pragma once

#include <atomic>

template<typename T>
struct node
{
	T data;
	std::atomic<node<T>*> next;

	explicit node(const T& data)
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
		// Инициализируем с dummy-узлом
		node<T>* dummy = new node<T>(T {});
		head.store(dummy, std::memory_order_relaxed);
		tail.store(dummy, std::memory_order_relaxed);
	}

	~lfQueue()
	{
		// Очистка памяти (в реальном коде нужно аккуратно обрабатывать многопоточность)
		while (node<T>* current = head.load())
		{
			head.store(current->next);
			delete current;
		}
	}

	void push(const T& data)
	{
		node<T>* new_node = new node<T>(data);

		while (true)
		{
			node<T>* current_tail = tail.load(std::memory_order_acquire);
			node<T>* next = current_tail->next.load(std::memory_order_acquire);

			// Проверяем, что tail не изменился
			if (current_tail != tail.load(std::memory_order_relaxed))
			{
				continue;
			}

			if (next == nullptr)
			{
				// Пытаемся добавить new_node в конец
				if (current_tail->next.compare_exchange_weak(next, new_node, std::memory_order_release, std::memory_order_relaxed))
				{
					// Успешно добавили, обновляем tail
					tail.compare_exchange_strong(current_tail, new_node, std::memory_order_release, std::memory_order_relaxed);
					break;
				}
			}
			else
			{
				// Помогаем другим потокам продвинуть tail
				tail.compare_exchange_strong(current_tail, next, std::memory_order_release, std::memory_order_relaxed);
			}
		}
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